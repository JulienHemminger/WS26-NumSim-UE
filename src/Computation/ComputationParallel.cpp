#include "Computation/ComputationParallel.h"

void ComputationParallel::initialize(int argc, char* argv[]) {
    // parse the parameters
    settings_.loadFromFile(argv[1]);
    settings_.printSettings();

    // get own partion depending on where in the domain the process lies
    partitioning_ = std::make_shared<Partitioning>();
    partitioning_->initialize(settings_.nCells);

    // calculate mesh width
    for (int i = 0; i < 2; i++) {
        meshWidth_[i] = settings_.physicalSize[i] / settings_.nCells[i];
    }

    // initialize discretization and solver
    if (settings_.useDonorCell) {
        discretization_ =
            std::make_shared<DonorCell>(partitioning_->nCellsLocal(), meshWidth_,
                                        partitioning_->ownPartitionNeighbours(), settings_.alpha);
    } else {
        discretization_ = std::make_shared<CentralDifferences>(
            partitioning_->nCellsLocal(), meshWidth_, partitioning_->ownPartitionNeighbours());
    }

    pressureSolver_ = std::make_unique<RedBlack>(
        discretization_, settings_.epsilon, settings_.maximumNumberOfIterations, partitioning_);

    // misc
    dt_ = 0.;

    // initialize output writer
    outputWriterParaviewParallel_ =
        std::make_unique<OutputWriterParaviewParallel>(discretization_, partitioning_);
    outputWriterTextParallel_ =
        std::make_unique<OutputWriterTextParallel>(discretization_, partitioning_);
}

void ComputationParallel::runSimulation() {
    double currentTime = 0;

    // profiling start
    double TOTAL_START_TIME = MPI_Wtime();

    // the steps correspond to the steps in our algorithm in the overleaf or docs/numsim-algos.tex
    while (currentTime < settings_.endTime) {
        // step 1: set the boundary values / exchange the final velocities at the borders
        applyBoundaryValues();

        // profiling start
        double START_TIME = MPI_Wtime();

        // step 2: compute time step width
        computeTimeStepWidthParallel(currentTime);

        // profiling sum
        partitioning_->DURATION_TIME_STEP += MPI_Wtime() - START_TIME;

        currentTime += dt_;

        // step 4: calculate F, G with first setting the boundary conditions of F, G (step 1)
        computePreliminaryVelocities();
        exchangePreliminaryVelocityHalosHorizontal();
        exchangePreliminaryVelocityHalosVertical();

        // step 5: compute the right hand side of the pressure equation
        computeRightHandSide();

        // step 6: solve the pressure equation
        solvePressure();

        // step 7: calculate the final velocities
        computeVelocities();
        exchangeVelocityHalosHorizontal();
        exchangeVelocityHalosVertical();

        // profiling start
        START_TIME = MPI_Wtime();

        //// step 9: write output
        // if (std::floor(currentTime) == currentTime || currentTime == settings_.endTime)  // TODO
        //{
        //     // std::cout << "Writing output..." << std::endl;
        //     outputWriterParaviewParallel_->writeFile(currentTime);
        //     outputWriterTextParallel_->writeFile(currentTime);
        // }

        outputWriterParaviewParallel_->writeFile(currentTime);
        outputWriterTextParallel_->writeFile(currentTime);

        // profiling sum
        partitioning_->DURATION_OUTPUT += MPI_Wtime() - START_TIME;
    }

    // profiling
    partitioning_->DURATION_TOTAL += MPI_Wtime() - TOTAL_START_TIME;
    double communication_time = partitioning_->DURATION_COMMUNICATION_REDUCE +
                                partitioning_->DURATION_COMMUNICATION_EXCHANGE;

    if (partitioning_->ownRankNo() == 0) {
        std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
        std::cout << "Duration of the computation: "
                  << partitioning_->DURATION_TOTAL - communication_time -
                         partitioning_->DURATION_OUTPUT
                  << std::endl;
        std::cout << "Duration of the total communication: " << communication_time
                  << " with the exchange " << std::endl
                  << partitioning_->DURATION_COMMUNICATION_EXCHANGE << " and the reduce "
                  << partitioning_->DURATION_COMMUNICATION_REDUCE << std::endl;
        std::cout << "Duration of the residual norm calculation: "
                  << partitioning_->DURATION_RESIDUAL_NORM << std::endl;
        std::cout << "Duration of the time step calculation: " << partitioning_->DURATION_TIME_STEP
                  << std::endl;
        std::cout << "Duration of the total run: " << partitioning_->DURATION_TOTAL << std::endl;
        std::cout << "Duration of the outputting: " << partitioning_->DURATION_OUTPUT << std::endl;
        std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
    }

    return;
}

void ComputationParallel::computeTimeStepWidthParallel(double currentTime) {
    // compute timestep for each subdomain
    computeTimeStepWidth();

    // use the minimum as global timestep
    double dt_global;
    double dt_local = dt_;

    // profiling start
    double START_TIME = MPI_Wtime();

    MPI_Allreduce(&dt_local, &dt_global, 1, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD);

    // profiling sum
    partitioning_->DURATION_COMMUNICATION_REDUCE += MPI_Wtime() - START_TIME;

    //// if necessary adapt so that every full second is reached
    // if (std::floor(currentTime + dt_global * 1.25) ==
    //     std::floor(currentTime) +
    //         1)  // increase dt_global to avoid numerically instable small time steps
    //{
    //     dt_global = (double)(std::floor(currentTime) + 1) -
    //                 currentTime;  // currentTime hits exactly next second
    // }

    // or if necessary set currentTime + dt_ to endTime
    if (currentTime + dt_global > settings_.endTime) {
        dt_global = settings_.endTime - currentTime;

        std::cout << std::endl;
        std::cout << "Final time step!" << std::endl;
    }

    dt_ = dt_global;
}

void ComputationParallel::applyBoundaryValues() {
    uvExchangeHorizontal();
    uvExchangeVertical();
}

void ComputationParallel::exchangeVelocityHalosHorizontal() {
    const int uCount = discretization_->uJEnd() - discretization_->uJBegin();
    const int vCount = discretization_->vJEnd() - discretization_->vJBegin();
    std::vector<double> send(uCount);
    std::vector<double> receive(uCount);
    int index = 0;
    for (int j = discretization_->uJBegin(); j < discretization_->uJEnd(); j++) {
        send[index++] = discretization_->u(discretization_->nCells()[0] - 1, j);
    }

    double startTime = MPI_Wtime();
    MPI_Sendrecv(send.data(), uCount, MPI_DOUBLE, partitioning_->ownRightNeighbour(), 10,
                 receive.data(), uCount, MPI_DOUBLE, partitioning_->ownLeftNeighbour(), 10,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    partitioning_->DURATION_COMMUNICATION_EXCHANGE += MPI_Wtime() - startTime;

    if (!partitioning_->ownPartitionContainsLeftBoundary()) {
        index = 0;
        for (int j = discretization_->uJBegin(); j < discretization_->uJEnd(); j++) {
            discretization_->u(discretization_->uIBegin(), j) = receive[index++];
        }
    }

    index = 0;
    for (int j = discretization_->uJBegin(); j < discretization_->uJEnd(); j++) {
        send[index++] = discretization_->u(0, j);
    }

    startTime = MPI_Wtime();
    MPI_Sendrecv(send.data(), uCount, MPI_DOUBLE, partitioning_->ownLeftNeighbour(), 11,
                 receive.data(), uCount, MPI_DOUBLE, partitioning_->ownRightNeighbour(), 11,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    partitioning_->DURATION_COMMUNICATION_EXCHANGE += MPI_Wtime() - startTime;

    if (!partitioning_->ownPartitionContainsRightBoundary()) {
        index = 0;
        for (int j = discretization_->uJBegin(); j < discretization_->uJEnd(); j++) {
            discretization_->u(discretization_->nCells()[0], j) = receive[index++];
        }
    }

    send.resize(vCount);
    receive.resize(vCount);
    index = 0;
    for (int j = discretization_->vJBegin(); j < discretization_->vJEnd(); j++) {
        send[index++] = discretization_->v(discretization_->nCells()[0] - 1, j);
    }

    startTime = MPI_Wtime();
    MPI_Sendrecv(send.data(), vCount, MPI_DOUBLE, partitioning_->ownRightNeighbour(), 12,
                 receive.data(), vCount, MPI_DOUBLE, partitioning_->ownLeftNeighbour(), 12,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    partitioning_->DURATION_COMMUNICATION_EXCHANGE += MPI_Wtime() - startTime;

    if (!partitioning_->ownPartitionContainsLeftBoundary()) {
        index = 0;
        for (int j = discretization_->vJBegin(); j < discretization_->vJEnd(); j++) {
            discretization_->v(discretization_->vIBegin(), j) = receive[index++];
        }
    }

    index = 0;
    for (int j = discretization_->vJBegin(); j < discretization_->vJEnd(); j++) {
        send[index++] = discretization_->v(0, j);
    }

    startTime = MPI_Wtime();
    MPI_Sendrecv(send.data(), vCount, MPI_DOUBLE, partitioning_->ownLeftNeighbour(), 13,
                 receive.data(), vCount, MPI_DOUBLE, partitioning_->ownRightNeighbour(), 13,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    partitioning_->DURATION_COMMUNICATION_EXCHANGE += MPI_Wtime() - startTime;

    if (!partitioning_->ownPartitionContainsRightBoundary()) {
        index = 0;
        for (int j = discretization_->vJBegin(); j < discretization_->vJEnd(); j++) {
            discretization_->v(discretization_->nCells()[0], j) = receive[index++];
        }
    }
}

void ComputationParallel::exchangeVelocityHalosVertical() {
    const int uCount = discretization_->uIEnd() - discretization_->uIBegin();
    const int vCount = discretization_->vIEnd() - discretization_->vIBegin();
    std::vector<double> send(uCount);
    std::vector<double> receive(uCount);
    int index = 0;
    for (int i = discretization_->uIBegin(); i < discretization_->uIEnd(); i++) {
        send[index++] = discretization_->u(i, discretization_->nCells()[1] - 1);
    }

    double startTime = MPI_Wtime();
    MPI_Sendrecv(send.data(), uCount, MPI_DOUBLE, partitioning_->ownTopNeighbour(), 20,
                 receive.data(), uCount, MPI_DOUBLE, partitioning_->ownBottomNeighbour(), 20,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    partitioning_->DURATION_COMMUNICATION_EXCHANGE += MPI_Wtime() - startTime;

    if (!partitioning_->ownPartitionContainsBottomBoundary()) {
        index = 0;
        for (int i = discretization_->uIBegin(); i < discretization_->uIEnd(); i++) {
            discretization_->u(i, discretization_->uJBegin()) = receive[index++];
        }
    }

    index = 0;
    for (int i = discretization_->uIBegin(); i < discretization_->uIEnd(); i++) {
        send[index++] = discretization_->u(i, 0);
    }

    startTime = MPI_Wtime();
    MPI_Sendrecv(send.data(), uCount, MPI_DOUBLE, partitioning_->ownBottomNeighbour(), 21,
                 receive.data(), uCount, MPI_DOUBLE, partitioning_->ownTopNeighbour(), 21,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    partitioning_->DURATION_COMMUNICATION_EXCHANGE += MPI_Wtime() - startTime;

    if (!partitioning_->ownPartitionContainsTopBoundary()) {
        index = 0;
        for (int i = discretization_->uIBegin(); i < discretization_->uIEnd(); i++) {
            discretization_->u(i, discretization_->nCells()[1]) = receive[index++];
        }
    }

    send.resize(vCount);
    receive.resize(vCount);
    index = 0;
    for (int i = discretization_->vIBegin(); i < discretization_->vIEnd(); i++) {
        send[index++] = discretization_->v(i, discretization_->nCells()[1] - 1);
    }

    startTime = MPI_Wtime();
    MPI_Sendrecv(send.data(), vCount, MPI_DOUBLE, partitioning_->ownTopNeighbour(), 22,
                 receive.data(), vCount, MPI_DOUBLE, partitioning_->ownBottomNeighbour(), 22,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    partitioning_->DURATION_COMMUNICATION_EXCHANGE += MPI_Wtime() - startTime;

    if (!partitioning_->ownPartitionContainsBottomBoundary()) {
        index = 0;
        for (int i = discretization_->vIBegin(); i < discretization_->vIEnd(); i++) {
            discretization_->v(i, discretization_->vJBegin()) = receive[index++];
        }
    }

    index = 0;
    for (int i = discretization_->vIBegin(); i < discretization_->vIEnd(); i++) {
        send[index++] = discretization_->v(i, 0);
    }

    startTime = MPI_Wtime();
    MPI_Sendrecv(send.data(), vCount, MPI_DOUBLE, partitioning_->ownBottomNeighbour(), 23,
                 receive.data(), vCount, MPI_DOUBLE, partitioning_->ownTopNeighbour(), 23,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    partitioning_->DURATION_COMMUNICATION_EXCHANGE += MPI_Wtime() - startTime;

    if (!partitioning_->ownPartitionContainsTopBoundary()) {
        index = 0;
        for (int i = discretization_->vIBegin(); i < discretization_->vIEnd(); i++) {
            discretization_->v(i, discretization_->nCells()[1]) = receive[index++];
        }
    }
}

void ComputationParallel::exchangePreliminaryVelocityHalosHorizontal() {
    const int jBegin = discretization_->uJBegin() + 1;
    const int jEnd = discretization_->uJEnd() - 1;
    const int count = jEnd - jBegin;
    std::vector<double> send(count);
    std::vector<double> receive(count);

    int index = 0;
    for (int j = jBegin; j < jEnd; j++) {
        send[index++] = discretization_->f(discretization_->nCells()[0] - 1, j);
    }

    const double startTime = MPI_Wtime();
    MPI_Sendrecv(send.data(), count, MPI_DOUBLE, partitioning_->ownRightNeighbour(), 12,
                 receive.data(), count, MPI_DOUBLE, partitioning_->ownLeftNeighbour(), 12,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    partitioning_->DURATION_COMMUNICATION_EXCHANGE += MPI_Wtime() - startTime;

    if (!partitioning_->ownPartitionContainsLeftBoundary()) {
        index = 0;
        for (int j = jBegin; j < jEnd; j++) {
            discretization_->f(discretization_->uIBegin(), j) = receive[index++];
        }
    }
}

void ComputationParallel::exchangePreliminaryVelocityHalosVertical() {
    const int iBegin = discretization_->vIBegin() + 1;
    const int iEnd = discretization_->vIEnd() - 1;
    const int count = iEnd - iBegin;
    std::vector<double> send(count);
    std::vector<double> receive(count);

    int index = 0;
    for (int i = iBegin; i < iEnd; i++) {
        send[index++] = discretization_->g(i, discretization_->nCells()[1] - 1);
    }

    const double startTime = MPI_Wtime();
    MPI_Sendrecv(send.data(), count, MPI_DOUBLE, partitioning_->ownTopNeighbour(), 13,
                 receive.data(), count, MPI_DOUBLE, partitioning_->ownBottomNeighbour(), 13,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    partitioning_->DURATION_COMMUNICATION_EXCHANGE += MPI_Wtime() - startTime;

    if (!partitioning_->ownPartitionContainsBottomBoundary()) {
        index = 0;
        for (int i = iBegin; i < iEnd; i++) {
            discretization_->g(i, discretization_->vJBegin()) = receive[index++];
        }
    }
}

void ComputationParallel::applyBoundaryValuesLeft() {
    //  u
    for (int j = discretization_->uJBegin(); j < discretization_->uJEnd(); j++) {
        discretization_->u(discretization_->uIBegin(), j) = settings_.dirichletBcLeft[0];
    }
    // v
    for (int j = discretization_->vJBegin(); j < discretization_->vJEnd(); j++) {
        discretization_->v(discretization_->vIBegin(), j) =
            2. * settings_.dirichletBcLeft[1] -
            discretization_->v(discretization_->vIBegin() + 1, j);
    }
    // f
    for (int j = discretization_->uJBegin(); j < discretization_->uJEnd(); j++) {
        discretization_->f(discretization_->uIBegin(), j) =
            discretization_->u(discretization_->uIBegin(), j);
    }
    // g
    for (int j = discretization_->vJBegin(); j < discretization_->vJEnd(); j++) {
        discretization_->g(discretization_->vIBegin(), j) =
            discretization_->v(discretization_->vIBegin(), j);
    }
}

void ComputationParallel::applyBoundaryValuesRight() {
    // u
    for (int j = discretization_->uJBegin(); j < discretization_->uJEnd(); j++) {
        discretization_->u(discretization_->uIEnd() - 1, j) = settings_.dirichletBcRight[0];
    }
    // v
    for (int j = discretization_->vJBegin(); j < discretization_->vJEnd(); j++) {
        discretization_->v(discretization_->vIEnd() - 1, j) =
            2. * settings_.dirichletBcRight[1] -
            discretization_->v(discretization_->vIEnd() - 2, j);
    }
    // f
    for (int j = discretization_->uJBegin(); j < discretization_->uJEnd(); j++) {
        discretization_->f(discretization_->uIEnd() - 1, j) =
            discretization_->u(discretization_->uIEnd() - 1, j);
    }
    // g
    for (int j = discretization_->vJBegin(); j < discretization_->vJEnd(); j++) {
        discretization_->g(discretization_->vIEnd() - 1, j) =
            discretization_->v(discretization_->vIEnd() - 1, j);
    }
}

void ComputationParallel::applyBoundaryValuesBottom() {
    const int iBegin = partitioning_->ownPartitionContainsLeftBoundary()
                           ? discretization_->uIBegin() + 1
                           : discretization_->uIBegin();
    for (int i = iBegin; i < discretization_->uIEnd() - 1; i++) {
        // u
        discretization_->u(i, discretization_->uJBegin()) =
            2. * settings_.dirichletBcBottom[0] -
            discretization_->u(i, discretization_->uJBegin() + 1);
        // f
        discretization_->f(i, discretization_->uJBegin()) =
            discretization_->u(i, discretization_->uJBegin());
    }
    for (int i = discretization_->vIBegin() + 1; i < discretization_->vIEnd() - 1; i++) {
        // v
        discretization_->v(i, discretization_->vJBegin()) = settings_.dirichletBcBottom[1];
        // g
        discretization_->g(i, discretization_->vJBegin()) =
            discretization_->v(i, discretization_->vJBegin());
    }
}

void ComputationParallel::applyBoundaryValuesTop() {
    const int iBegin = partitioning_->ownPartitionContainsLeftBoundary()
                           ? discretization_->uIBegin() + 1
                           : discretization_->uIBegin();
    for (int i = iBegin; i < discretization_->uIEnd() - 1; i++) {
        // u
        discretization_->u(i, discretization_->uJEnd() - 1) =
            2. * settings_.dirichletBcTop[0] - discretization_->u(i, discretization_->uJEnd() - 2);
        // f
        discretization_->f(i, discretization_->uJEnd() - 1) =
            discretization_->u(i, discretization_->uJEnd() - 1);
    }
    for (int i = discretization_->vIBegin() + 1; i < discretization_->vIEnd() - 1; i++) {
        // v
        discretization_->v(i, discretization_->vJEnd() - 1) = settings_.dirichletBcTop[1];
        // g
        discretization_->g(i, discretization_->vJEnd() - 1) =
            discretization_->v(i, discretization_->vJEnd() - 1);
    }
}

void ComputationParallel::uvExchangeHorizontal() {
    if (partitioning_->ownPartitionContainsLeftBoundary()) applyBoundaryValuesLeft();
    if (partitioning_->ownPartitionContainsRightBoundary()) applyBoundaryValuesRight();
    exchangeVelocityHalosHorizontal();
}

void ComputationParallel::uvExchangeVertical() {
    if (partitioning_->ownPartitionContainsBottomBoundary()) applyBoundaryValuesBottom();
    if (partitioning_->ownPartitionContainsTopBoundary()) applyBoundaryValuesTop();
    exchangeVelocityHalosVertical();
}
