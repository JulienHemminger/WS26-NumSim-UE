#include "Computation.h"

void Computation::initialize(int argc, char* argv[]) {
    settings_.loadFromFile(argv[1]);
    settings_.printSettings();

    for (int i = 0; i < 2; i++) {
        meshWidth_[i] = settings_.physicalSize[i] / settings_.nCells[i];
        std::cout << "computed mesh width " << i << ": " << meshWidth_[i] << " "
                  << settings_.physicalSize[i] << " " << settings_.nCells[i] << std::endl;
    }

    if (settings_.useDonorCell) {
        discretization_ =
            std::make_shared<DonorCell>(settings_.nCells, meshWidth_, settings_.alpha);
    } else {
        discretization_ = std::make_shared<CentralDifferences>(settings_.nCells, meshWidth_);
    }

    if (settings_.pressureSolver == "SOR") {
        pressureSolver_ = std::make_unique<SOR>(
            discretization_, settings_.epsilon, settings_.maximumNumberOfIterations,
            settings_.omega, settings_.outflowBottom, settings_.outflowTop, settings_.outflowLeft,
            settings_.outflowRight);
    } else if (settings_.pressureSolver == "GaussSeidel") {
        pressureSolver_ = std::make_unique<GaussSeidel>(
            discretization_, settings_.epsilon, settings_.maximumNumberOfIterations,
            settings_.outflowBottom, settings_.outflowTop, settings_.outflowLeft,
            settings_.outflowRight);
    } else {
        std::cout << "The name of the pressure solver is not understood" << settings_.pressureSolver
                  << std::endl;
        throw;
    }

    dt_ = 0.;

    outputWriterParaview_ = std::make_unique<OutputWriterParaview>(discretization_, settings_.uID);
    outputWriterText_ = std::make_unique<OutputWriterText>(discretization_, settings_.uID);

    if (settings_.complexGeometryPath != "None") {
        discretization_->setObstacleFlags(settings_.complexGeometryPath);
    }

    discretization_->setObstacleNeighbourFlags();
}

void Computation::runSimulation() {
    double currentTime = 0;

    std::cout << "+++++++++++++++++++++++" << std::endl;
    std::cout << "Starting at time: " << currentTime << std::endl;
    std::cout << "+++++++++++++++++++++++" << std::endl;

    while (currentTime < settings_.endTime) {
        applyBoundaryValues();
        std::cout << "Applied boundary values for u/v and F/G." << std::endl;

        applyObstacleBoundaryValues();
        std::cout << "Applied obstacle boundary values for u/v and F/G." << std::endl;

        computeTimeStepWidth(currentTime);

        std::cout << "Computed time step width: " << dt_ << std::endl;
        currentTime += dt_;

        std::cout << "+++++++++++++++++++++++" << std::endl;
        std::cout << "current Time: " << currentTime << std::endl;
        std::cout << "+++++++++++++++++++++++" << std::endl;

        computePreliminaryVelocities();

        std::cout << "Computed preliminary velocities" << std::endl;

        computeRightHandSide();

        std::cout << "Computed right hand side" << std::endl;

        computePressure();

        std::cout << "Computed pressure" << std::endl;

        computeVelocities();

        std::cout << "Computed velocities" << std::endl;

        std::cout << "Writing output..." << std::endl;
        outputWriterParaview_->writeFile(currentTime);
        outputWriterText_->writeFile(currentTime);
    }

    std::cout << "Simulation finished!!" << std::endl;
}

void Computation::computeTimeStepWidth(double currentTime) {
    double boundary_diffusion = 0.;

    if (meshWidth_[0] == meshWidth_[1]) {
        boundary_diffusion = (settings_.re * meshWidth_[0] * meshWidth_[1]) / 4.;
    } else {
        double h2x = meshWidth_[0] * meshWidth_[0];
        double h2y = meshWidth_[1] * meshWidth_[1];
        boundary_diffusion = (settings_.re / 2.) * (h2x * h2y) * (1. / (h2x + h2y));

        std::cout << settings_.re << " " << h2x << " " << h2y << std::endl;
    }

    double u_max = 0.;
    for (int j = discretization_->uJBegin(); j < discretization_->uJEnd(); j++) {
        for (int i = discretization_->uIBegin(); i < discretization_->uIEnd(); i++) {
            double value = std::abs(discretization_->u(i, j));
            if (value > u_max) {
                u_max = value;
            }
        }
    }

    double v_max = 0.;
    for (int j = discretization_->vJBegin(); j < discretization_->vJEnd(); j++) {
        for (int i = discretization_->vIBegin(); i < discretization_->vIEnd(); i++) {
            double value = std::abs(discretization_->v(i, j));
            if (value > v_max) {
                v_max = value;
            }
        }
    }

    double boundary_convection_u = meshWidth_[0] / u_max;
    double boundary_convection_v = meshWidth_[1] / v_max;

    double min_dt = std::min({boundary_diffusion, boundary_convection_u, boundary_convection_v});

    std::cout << "dt boundaries - diffusion: " << boundary_diffusion
              << " convection_u: " << boundary_convection_u
              << " convection_v: " << boundary_convection_v << std::endl;

    dt_ = std::min(min_dt * settings_.tau, settings_.maximumDt);

    if (currentTime + dt_ > settings_.endTime) {
        dt_ = settings_.endTime - currentTime;

        std::cout << std::endl;
        std::cout << "Final time step!" << std::endl;
    }
}

void Computation::applyBoundaryValues() {
    if (settings_.outflowTop) {
        for (int i = discretization_->uIBegin(); i < discretization_->uIEnd(); i++) {
            discretization_->u(i, discretization_->uJEnd() - 1) =
                discretization_->u(i, discretization_->uJEnd() - 2);
        }

        for (int i = discretization_->vIBegin(); i < discretization_->vIEnd(); i++) {
            discretization_->v(i, discretization_->vJEnd() - 1) =
                discretization_->v(i, discretization_->vJEnd() - 2);
            discretization_->g(i, discretization_->vJEnd() - 1) =
                discretization_->v(i, discretization_->vJEnd() - 1);
        }

    } else {
        for (int i = discretization_->uIBegin(); i < discretization_->uIEnd(); i++) {
            discretization_->u(i, discretization_->uJEnd() - 1) =
                2. * settings_.dirichletBcTop[0] -
                discretization_->u(i, discretization_->uJEnd() - 2);
            discretization_->f(i, discretization_->uJEnd() - 1) =
                discretization_->u(i, discretization_->uJEnd() - 1);
        }

        for (int i = discretization_->vIBegin(); i < discretization_->vIEnd(); i++) {
            discretization_->v(i, discretization_->vJEnd() - 1) = settings_.dirichletBcTop[1];
            discretization_->g(i, discretization_->vJEnd() - 1) =
                discretization_->v(i, discretization_->vJEnd() - 1);
        }
    }

    if (settings_.outflowBottom) {
        for (int i = discretization_->uIBegin(); i < discretization_->uIEnd(); i++) {
            discretization_->u(i, discretization_->uJBegin()) =
                discretization_->u(i, discretization_->uJBegin() + 1);
        }

        for (int i = discretization_->vIBegin(); i < discretization_->vIEnd(); i++) {
            discretization_->v(i, discretization_->vJBegin()) =
                discretization_->v(i, discretization_->vJBegin() + 1);
            discretization_->g(i, discretization_->vJBegin()) =
                discretization_->v(i, discretization_->vJBegin());
        }
    } else {
        for (int i = discretization_->uIBegin(); i < discretization_->uIEnd(); i++) {
            discretization_->u(i, discretization_->uJBegin()) =
                2. * settings_.dirichletBcBottom[0] -
                discretization_->u(i, discretization_->uJBegin() + 1);
            discretization_->f(i, discretization_->uJBegin()) =
                discretization_->u(i, discretization_->uJBegin());
        }

        for (int i = discretization_->vIBegin(); i < discretization_->vIEnd(); i++) {
            discretization_->v(i, discretization_->vJBegin()) = settings_.dirichletBcBottom[1];
            discretization_->g(i, discretization_->vJBegin()) =
                discretization_->v(i, discretization_->vJBegin());
        }
    }

    if (settings_.outflowLeft) {
        for (int j = discretization_->uJBegin(); j < discretization_->uJEnd(); j++) {
            discretization_->u(discretization_->uIBegin(), j) =
                discretization_->u(discretization_->uIBegin() + 1, j);
            discretization_->f(discretization_->uIBegin(), j) =
                discretization_->u(discretization_->uIBegin(), j);
        }

        for (int j = discretization_->vJBegin(); j < discretization_->vJEnd(); j++) {
            discretization_->v(discretization_->vIBegin(), j) =
                discretization_->v(discretization_->vIBegin() + 1, j);
        }
    } else {
        for (int j = discretization_->uJBegin(); j < discretization_->uJEnd(); j++) {
            discretization_->u(discretization_->uIBegin(), j) = settings_.dirichletBcLeft[0];
            discretization_->f(discretization_->uIBegin(), j) =
                discretization_->u(discretization_->uIBegin(), j);
        }

        for (int j = discretization_->vJBegin(); j < discretization_->vJEnd(); j++) {
            discretization_->v(discretization_->vIBegin(), j) =
                2. * settings_.dirichletBcLeft[1] -
                discretization_->v(discretization_->vIBegin() + 1, j);
            discretization_->g(discretization_->vIBegin(), j) =
                discretization_->v(discretization_->vIBegin(), j);
        }
    }

    if (settings_.outflowRight) {
        for (int j = discretization_->uJBegin(); j < discretization_->uJEnd(); j++) {
            discretization_->u(discretization_->uIEnd() - 1, j) =
                discretization_->u(discretization_->uIEnd() - 2, j);
            discretization_->f(discretization_->uIEnd() - 1, j) =
                discretization_->u(discretization_->uIEnd() - 1, j);
        }

        for (int j = discretization_->vJBegin(); j < discretization_->vJEnd(); j++) {
            discretization_->v(discretization_->vIEnd() - 1, j) =
                discretization_->v(discretization_->vIEnd() - 2, j);
        }
    } else {
        for (int j = discretization_->uJBegin(); j < discretization_->uJEnd(); j++) {
            discretization_->u(discretization_->uIEnd() - 1, j) = settings_.dirichletBcRight[0];
            discretization_->f(discretization_->uIEnd() - 1, j) =
                discretization_->u(discretization_->uIEnd() - 1, j);
        }

        for (int j = discretization_->vJBegin(); j < discretization_->vJEnd(); j++) {
            discretization_->v(discretization_->vIEnd() - 1, j) =
                2. * settings_.dirichletBcRight[1] -
                discretization_->v(discretization_->vIEnd() - 2, j);
            discretization_->g(discretization_->vIEnd() - 1, j) =
                discretization_->v(discretization_->vIEnd() - 1, j);
        }
    }
}

void Computation::applyObstacleBoundaryValues() {
    for (int i = 0; i < discretization_->nCells()[0]; i++) {
        for (int j = 0; j < discretization_->nCells()[1]; j++) {
            if (discretization_->isObstacleCell(i, j) != 1) {
                continue;
            }

            bool hasLeftNeighbour = discretization_->hasFluidNeighbourLeft(i, j) == 1;
            bool hasRightNeighbour = discretization_->hasFluidNeighbourRight(i, j) == 1;
            bool hasTopNeighbour = discretization_->hasFluidNeighbourTop(i, j) == 1;
            bool hasBottomNeighbour = discretization_->hasFluidNeighbourBottom(i, j) == 1;

            if (hasLeftNeighbour) {
                discretization_->u(i - 1, j) = 0;
                discretization_->f(i - 1, j) = 0;

                if (hasTopNeighbour) {
                    // Left and top
                    discretization_->u(i, j) = -discretization_->u(i, j + 1);
                    discretization_->v(i, j - 1) = -discretization_->v(i - 1, j - 1);
                } else if (hasBottomNeighbour) {
                    // Left and bottom
                    discretization_->u(i, j) = -discretization_->u(i, j - 1);
                    discretization_->v(i, j) = -discretization_->v(i - 1, j);
                } else {
                    // Only left
                    discretization_->v(i, j - 1) = -discretization_->v(i - 1, j - 1);
                    discretization_->v(i, j) = -discretization_->v(i - 1, j);
                }
            }
            if (hasRightNeighbour) {
                discretization_->u(i, j) = 0;
                discretization_->f(i, j) = 0;
                if (hasTopNeighbour) {
                    // Right and top
                    discretization_->u(i - 1, j) = -discretization_->u(i - 1, j + 1);
                    discretization_->v(i, j - 1) = -discretization_->v(i + 1, j - 1);
                } else if (hasBottomNeighbour) {
                    // Right and bottom
                    discretization_->u(i - 1, j) = -discretization_->u(i - 1, j - 1);
                    discretization_->v(i, j) = -discretization_->v(i + 1, j);
                } else {
                    // Only right
                    discretization_->v(i, j) = -discretization_->v(i + 1, j);
                    discretization_->v(i, j - 1) = -discretization_->v(i + 1, j - 1);
                }
            }

            if (hasTopNeighbour) {
                discretization_->v(i, j) = 0;
                discretization_->g(i, j) = 0;
                if (!hasLeftNeighbour && !hasRightNeighbour) {
                    // Only top
                    discretization_->u(i, j) = -discretization_->u(i, j + 1);
                    discretization_->u(i - 1, j) = -discretization_->u(i - 1, j + 1);
                }
            }
            if (hasBottomNeighbour) {
                discretization_->v(i, j - 1) = 0;
                discretization_->g(i, j - 1) = 0;
                if (!hasLeftNeighbour && !hasRightNeighbour) {
                    // Only bottom
                    discretization_->u(i, j) = -discretization_->u(i, j - 1);
                    discretization_->u(i - 1, j) = -discretization_->u(i - 1, j - 1);
                }
            }
        }
    }
}

void Computation::computePreliminaryVelocities() {
    for (int j = discretization_->uJBegin() + 1; j < discretization_->uJEnd() - 1; j++) {
        for (int i = discretization_->uIBegin() + 1; i < discretization_->uIEnd() - 1; i++) {
            if ((discretization_->isObstacleCell(i, j) != 1.) &&
                (discretization_->hasFluidNeighbourLeft(i + 1, j) != 1.)) {
                double diffusion =
                    discretization_->computeD2uDx2(i, j) + discretization_->computeD2uDy2(i, j);
                double convection =
                    -discretization_->computeDu2Dx(i, j) - discretization_->computeDuvDy(i, j);
                double sum = ((1. / settings_.re) * diffusion) + convection + settings_.g[0];
                discretization_->f(i, j) = discretization_->u(i, j) + dt_ * sum;
            }
        }
    }

    for (int j = discretization_->vJBegin() + 1; j < discretization_->vJEnd() - 1; j++) {
        for (int i = discretization_->vIBegin() + 1; i < discretization_->vIEnd() - 1; i++) {
            if ((discretization_->isObstacleCell(i, j) != 1.) &&
                (discretization_->hasFluidNeighbourBottom(i, j + 1) != 1.)) {
                double diffusion =
                    discretization_->computeD2vDx2(i, j) + discretization_->computeD2vDy2(i, j);
                double convection =
                    -discretization_->computeDuvDx(i, j) - discretization_->computeDv2Dy(i, j);
                double sum = ((1. / settings_.re) * diffusion) + convection + settings_.g[1];
                discretization_->g(i, j) = discretization_->v(i, j) + dt_ * sum;
            }
        }
    }
}

void Computation::computeRightHandSide() {
    for (int j = discretization_->pJBegin() + 1; j < discretization_->pJEnd() - 1; j++) {
        for (int i = discretization_->pIBegin() + 1; i < discretization_->pIEnd() - 1; i++) {
            if (discretization_->isObstacleCell(i, j) != 1.) {
                double difference_f = (discretization_->f(i, j) - discretization_->f(i - 1, j)) /
                                      discretization_->dx();
                double difference_g = (discretization_->g(i, j) - discretization_->g(i, j - 1)) /
                                      discretization_->dy();
                discretization_->rhs(i, j) = (1 / dt_) * (difference_f + difference_g);
            }
        }
    }
}

void Computation::computePressure() { pressureSolver_->solve(); }

void Computation::computeVelocities() {
    for (int j = discretization_->uJBegin() + 1; j < discretization_->uJEnd() - 1; j++) {
        for (int i = discretization_->uIBegin() + 1; i < discretization_->uIEnd() - 1; i++) {
            if ((discretization_->isObstacleCell(i, j) != 1.) &&
                (discretization_->hasFluidNeighbourLeft(i + 1, j) != 1.)) {
                discretization_->u(i, j) =
                    (1 - settings_.underrelaxationVelocity) * discretization_->u(i, j) +
                    settings_.underrelaxationVelocity *
                        (discretization_->f(i, j) - dt_ * discretization_->computeDpDx(i, j));
            }
        }
    }

    for (int j = discretization_->vJBegin() + 1; j < discretization_->vJEnd() - 1; j++) {
        for (int i = discretization_->vIBegin() + 1; i < discretization_->vIEnd() - 1; i++) {
            if ((discretization_->isObstacleCell(i, j) != 1.) &&
                (discretization_->hasFluidNeighbourBottom(i, j + 1) != 1.)) {
                discretization_->v(i, j) =
                    (1 - settings_.underrelaxationVelocity) * discretization_->v(i, j) +
                    settings_.underrelaxationVelocity *
                        (discretization_->g(i, j) - dt_ * discretization_->computeDpDy(i, j));
            }
        }
    }
}

void Computation::runTest() {
    std::cout << settings_.pressureSolver << " " << settings_.useDonorCell << std::endl;

    std::cout << "+++++++++++++++++++++++++++++++++++ TASK 1" << std::endl;
    int fieldWidth = 9;
    for (int j = discretization_->pJEnd() - 1; j >= discretization_->pJBegin(); j--) {
        std::cout << std::setw(fieldWidth) << j << "|";
        for (int i = discretization_->pIBegin(); i < discretization_->pIEnd(); i++) {
            std::cout << std::setw(fieldWidth) << std::setprecision(fieldWidth - 6)
                      << discretization_->p(i, j);
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    for (int j = discretization_->pJBegin() + 1; j < discretization_->pJEnd() - 1; j++) {
        for (int i = discretization_->pIBegin() + 1; i < discretization_->pIEnd() - 1; i++) {
            discretization_->rhs(i, j) = 0;
            discretization_->p(i, j) = 1;
        }
    }
    std::cout << "+++++++++++++++++++++++++++++++++++" << std::endl;

    pressureSolver_->solve();
    for (int j = discretization_->pJEnd() - 1; j >= discretization_->pJBegin(); j--) {
        std::cout << std::setw(fieldWidth) << j << "|";
        for (int i = discretization_->pIBegin(); i < discretization_->pIEnd(); i++) {
            std::cout << std::setw(fieldWidth) << std::setprecision(fieldWidth - 6)
                      << discretization_->p(i, j);
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    std::cout << settings_.pressureSolver << " " << settings_.useDonorCell << std::endl;

    std::cout << "+++++++++++++++++++++++++++++++++++ TASK 2" << std::endl;
    for (int j = discretization_->pJEnd() - 1; j >= discretization_->pJBegin(); j--) {
        std::cout << std::setw(fieldWidth) << j << "|";
        for (int i = discretization_->pIBegin(); i < discretization_->pIEnd(); i++) {
            std::cout << std::setw(fieldWidth) << std::setprecision(fieldWidth - 6)
                      << discretization_->p(i, j);
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    for (int j = discretization_->pJBegin() + 1; j < discretization_->pJEnd() - 1; j++) {
        for (int i = discretization_->pIBegin() + 1; i < discretization_->pIEnd() - 1; i++) {
            discretization_->rhs(i, j) = 1;
            discretization_->p(i, j) = 0;
        }
    }
    std::cout << "+++++++++++++++++++++++++++++++++++" << std::endl;

    pressureSolver_->solve();
    for (int j = discretization_->pJEnd() - 1; j >= discretization_->pJBegin(); j--) {
        std::cout << std::setw(fieldWidth) << j << "|";
        for (int i = discretization_->pIBegin(); i < discretization_->pIEnd(); i++) {
            std::cout << std::setw(fieldWidth) << std::setprecision(fieldWidth - 6)
                      << discretization_->p(i, j);
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}