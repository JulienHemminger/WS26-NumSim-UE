#include "Computation.h"

void Computation::initialize(int argc, char* argv[]) {
    settings_.loadFromFile(argv[1]);
    settings_.printSettings();

    for (int i = 0; i < 2; i++) {
        meshWidth_[i] = settings_.physicalSize[i] / settings_.nCells[i];
    }

    if (settings_.useDonorCell) {
        discretization_ =
            std::make_shared<DonorCell>(settings_.nCells, meshWidth_, settings_.alpha);
    } else {
        discretization_ = std::make_shared<CentralDifferences>(settings_.nCells, meshWidth_);
    }

    if (settings_.pressureSolver == "SOR") {
        pressureSolver_ =
            std::make_unique<SOR>(discretization_, settings_.epsilon,
                                  settings_.maximumNumberOfIterations, settings_.omega);
    } else if (settings_.pressureSolver == "GaussSeidel") {
        pressureSolver_ = std::make_unique<GaussSeidel>(discretization_, settings_.epsilon,
                                                        settings_.maximumNumberOfIterations);
    } else {
        std::cout << "Invalid pressure solver name: " << settings_.pressureSolver << std::endl;
        throw;
    }

    dt_ = 0.;

    outputWriterParaview_ = std::make_unique<OutputWriterParaview>(discretization_);
    outputWriterText_ = std::make_unique<OutputWriterText>(discretization_);
}

void Computation::runSimulation() {
    double currentTime = 0;

    std::cout << "Starting Time: " << currentTime << std::endl;

    // see Chapter 3.2.6 "Algorithm for One Time Step"
    while (currentTime < settings_.endTime) {
        applyBoundaryValues();

        computeTimeStepWidth();
        // Checks for the last time step
        dt_ = std::min(dt_, settings_.endTime - currentTime);
        currentTime += dt_;

        std::cout << "Current Time: " << currentTime << " with time step " << dt_ << std::endl;

        computePreliminaryVelocities();

        computeRightHandSide();

        solvePressure();

        computeVelocities();

        outputWriterParaview_->writeFile(currentTime);
        outputWriterText_->writeFile(currentTime);
    }

    std::cout << "Simulation completed" << std::endl;
}

void Computation::computeTimeStepWidth() {
    // Compute the biggest possible time step which is still stable. See Chapter 3.2.5 "Time Step –
    // Stability Conditions"

    // Time step condition by diffusion operator (Reynolds number)
    double boundary_diffusion = 0.;

    if (meshWidth_[0] == meshWidth_[1]) {
        boundary_diffusion = (settings_.re * meshWidth_[0] * meshWidth_[1]) / 4.;
    } else {
        double h2x = meshWidth_[0] * meshWidth_[0];
        double h2y = meshWidth_[1] * meshWidth_[1];
        boundary_diffusion = (settings_.re / 2.) * (h2x * h2y) * (1. / (h2x + h2y));
    }

    // Time step condition by convection operator in x/y direction with highest velocity
    const auto maximumAbsoluteVelocity = [](int iBegin, int iEnd, int jBegin, int jEnd,
                                            const auto& velocityAt) {
        double maximum = 0.;
        for (int j = jBegin; j < jEnd; j++) {
            for (int i = iBegin; i < iEnd; i++) {
                maximum = std::max(maximum, std::abs(velocityAt(i, j)));
            }
        }
        return maximum;
    };

    const double u_max = maximumAbsoluteVelocity(
        discretization_->uIBegin(), discretization_->uIEnd(), discretization_->uJBegin(),
        discretization_->uJEnd(), [&](int i, int j) { return discretization_->u(i, j); });
    const double v_max = maximumAbsoluteVelocity(
        discretization_->vIBegin(), discretization_->vIEnd(), discretization_->vJBegin(),
        discretization_->vJEnd(), [&](int i, int j) { return discretization_->v(i, j); });

    double boundary_convection_u = meshWidth_[0] / u_max;
    double boundary_convection_v = meshWidth_[1] / v_max;

    // The smallest necessary time step is the important one
    double min_dt = std::min({boundary_diffusion, boundary_convection_u, boundary_convection_v});

    dt_ = std::min(min_dt * settings_.tau, settings_.maximumDt);
}

void Computation::applyBoundaryValues() {
    // Apply boundary values to the velocity fields
    // On a staggered grid, the tangential velocity is interpolated at the boundary.

    const auto setHorizontalBoundary = [&](bool top, const std::array<double, 2>& boundaryValues) {
        const int uBoundary = top ? discretization_->uJEnd() - 1 : discretization_->uJBegin();
        const int uInterior = top ? uBoundary - 1 : uBoundary + 1;
        const int vBoundary = top ? discretization_->vJEnd() - 1 : discretization_->vJBegin();

        for (int i = discretization_->uIBegin(); i < discretization_->uIEnd(); i++) {
            discretization_->u(i, uBoundary) =
                2. * boundaryValues[0] - discretization_->u(i, uInterior);
            discretization_->f(i, uBoundary) = discretization_->u(i, uBoundary);
        }

        for (int i = discretization_->vIBegin(); i < discretization_->vIEnd(); i++) {
            discretization_->v(i, vBoundary) = boundaryValues[1];
            discretization_->g(i, vBoundary) = discretization_->v(i, vBoundary);
        }
    };

    setHorizontalBoundary(true, settings_.dirichletBcTop);
    setHorizontalBoundary(false, settings_.dirichletBcBottom);

    const auto setVerticalBoundary = [&](bool left, const std::array<double, 2>& boundaryValues) {
        const int uBoundary = left ? discretization_->uIBegin() : discretization_->uIEnd() - 1;
        const int vBoundary = left ? discretization_->vIBegin() : discretization_->vIEnd() - 1;
        const int vInterior = left ? vBoundary + 1 : vBoundary - 1;

        for (int j = discretization_->uJBegin(); j < discretization_->uJEnd(); j++) {
            discretization_->u(uBoundary, j) = boundaryValues[0];
            discretization_->f(uBoundary, j) = discretization_->u(uBoundary, j);
        }

        for (int j = discretization_->vJBegin(); j < discretization_->vJEnd(); j++) {
            discretization_->v(vBoundary, j) =
                2. * boundaryValues[1] - discretization_->v(vInterior, j);
            discretization_->g(vBoundary, j) = discretization_->v(vBoundary, j);
        }
    };

    setVerticalBoundary(true, settings_.dirichletBcLeft);
    setVerticalBoundary(false, settings_.dirichletBcRight);
}

void Computation::computePreliminaryVelocities() {
    // Computes the preliminary velocities f and g which will be used for the pressure equation.
    // This uses equation (3.2).
    for (int j = discretization_->uJBegin() + 1; j < discretization_->uJEnd() - 1; j++) {
        for (int i = discretization_->uIBegin() + 1; i < discretization_->uIEnd() - 1; i++) {
            double diffusion =
                discretization_->computeD2uDx2(i, j) + discretization_->computeD2uDy2(i, j);
            double convection =
                -discretization_->computeDu2Dx(i, j) - discretization_->computeDuvDy(i, j);
            double sum = ((1. / settings_.re) * diffusion) + convection + settings_.g[0];
            discretization_->f(i, j) = discretization_->u(i, j) + dt_ * sum;
        }
    }

    for (int j = discretization_->vJBegin() + 1; j < discretization_->vJEnd() - 1; j++) {
        for (int i = discretization_->vIBegin() + 1; i < discretization_->vIEnd() - 1; i++) {
            double diffusion =
                discretization_->computeD2vDx2(i, j) + discretization_->computeD2vDy2(i, j);
            double convection =
                -discretization_->computeDuvDx(i, j) - discretization_->computeDv2Dy(i, j);
            double sum = ((1. / settings_.re) * diffusion) + convection + settings_.g[1];
            discretization_->g(i, j) = discretization_->v(i, j) + dt_ * sum;
        }
    }
}

void Computation::computeRightHandSide() {
    // Computes the right hand side of the pressure equation. This is only necessary once per time
    // step and therefore not part of the pressure iteration. This uses equation (3.3).
    for (int j = discretization_->pJBegin() + 1; j < discretization_->pJEnd() - 1; j++) {
        for (int i = discretization_->pIBegin() + 1; i < discretization_->pIEnd() - 1; i++) {
            double difference_f =
                (discretization_->f(i, j) - discretization_->f(i - 1, j)) / discretization_->dx();
            double difference_g =
                (discretization_->g(i, j) - discretization_->g(i, j - 1)) / discretization_->dy();
            discretization_->rhs(i, j) = (1 / dt_) * (difference_f + difference_g);
        }
    }
}

void Computation::solvePressure() { pressureSolver_->solve(); }

void Computation::computeVelocities() {
    // Compute the final velocities. This is done after the pressure iteration, to gain velocities
    // fulfilling the continuum equation. This uses equations before (3.4).
    for (int j = discretization_->uJBegin() + 1; j < discretization_->uJEnd() - 1; j++) {
        for (int i = discretization_->uIBegin() + 1; i < discretization_->uIEnd() - 1; i++) {
            discretization_->u(i, j) =
                discretization_->f(i, j) - dt_ * discretization_->computeDpDx(i, j);
        }
    }

    for (int j = discretization_->vJBegin() + 1; j < discretization_->vJEnd() - 1; j++) {
        for (int i = discretization_->vIBegin() + 1; i < discretization_->vIEnd() - 1; i++) {
            discretization_->v(i, j) =
                discretization_->g(i, j) - dt_ * discretization_->computeDpDy(i, j);
        }
    }
}
