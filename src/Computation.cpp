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
        std::cout << "Invalid pressure solver name: " << settings_.pressureSolver << std::endl;
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

    std::cout << "Starting Time: " << currentTime << std::endl;

    while (currentTime < settings_.endTime) {
        applyBoundaryValues();

        applyObstacleBoundaryValues();

        computeTimeStepWidth();
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
    // Compute the biggest possible time step which is still stable

    // Time step condition by diffusion (Reynolds number)
    double boundary_diffusion = 0.;

    if (meshWidth_[0] == meshWidth_[1]) {
        boundary_diffusion = (settings_.re * meshWidth_[0] * meshWidth_[1]) / 4.;
    } else {
        double h2x = meshWidth_[0] * meshWidth_[0];
        double h2y = meshWidth_[1] * meshWidth_[1];
        boundary_diffusion = (settings_.re / 2.) * (h2x * h2y) * (1. / (h2x + h2y));
    }

    // Time step condition by convection in x/y direction with highest velocity
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

    // If an outflow is present in a direction, set the last velocities in each row/column to the
    // velocity of the outflow. This avoids jumps
    // If no outflow is present in a direction, a wall is present and standard Dirichlet
    // conditions can be applied. They need to be interpolated, as we are using a staggered grid and
    // u and v may not be given at the same positions as the wall

    const auto setHorizontalBoundary = [&](bool top, bool outflow,
                                           const std::array<double, 2>& boundaryValues) {
        const int uBoundary = top ? discretization_->uJEnd() - 1 : discretization_->uJBegin();
        const int uInterior = top ? uBoundary - 1 : uBoundary + 1;
        const int vBoundary = top ? discretization_->vJEnd() - 1 : discretization_->vJBegin();
        const int vInterior = top ? vBoundary - 1 : vBoundary + 1;

        for (int i = discretization_->uIBegin(); i < discretization_->uIEnd(); i++) {
            if (outflow) {
                discretization_->u(i, uBoundary) = discretization_->u(i, uInterior);
            } else {
                discretization_->u(i, uBoundary) =
                    2. * boundaryValues[0] - discretization_->u(i, uInterior);
                discretization_->f(i, uBoundary) = discretization_->u(i, uBoundary);
            }
        }

        for (int i = discretization_->vIBegin(); i < discretization_->vIEnd(); i++) {
            discretization_->v(i, vBoundary) =
                outflow ? discretization_->v(i, vInterior) : boundaryValues[1];
            discretization_->g(i, vBoundary) = discretization_->v(i, vBoundary);
        }
    };

    setHorizontalBoundary(true, settings_.outflowTop, settings_.dirichletBcTop);
    setHorizontalBoundary(false, settings_.outflowBottom, settings_.dirichletBcBottom);

    const auto setVerticalBoundary = [&](bool left, bool outflow,
                                         const std::array<double, 2>& boundaryValues) {
        const int uBoundary = left ? discretization_->uIBegin() : discretization_->uIEnd() - 1;
        const int uInterior = left ? uBoundary + 1 : uBoundary - 1;
        const int vBoundary = left ? discretization_->vIBegin() : discretization_->vIEnd() - 1;
        const int vInterior = left ? vBoundary + 1 : vBoundary - 1;

        for (int j = discretization_->uJBegin(); j < discretization_->uJEnd(); j++) {
            discretization_->u(uBoundary, j) =
                outflow ? discretization_->u(uInterior, j) : boundaryValues[0];
            discretization_->f(uBoundary, j) = discretization_->u(uBoundary, j);
        }

        for (int j = discretization_->vJBegin(); j < discretization_->vJEnd(); j++) {
            if (outflow) {
                discretization_->v(vBoundary, j) = discretization_->v(vInterior, j);
            } else {
                discretization_->v(vBoundary, j) =
                    2. * boundaryValues[1] - discretization_->v(vInterior, j);
                discretization_->g(vBoundary, j) = discretization_->v(vBoundary, j);
            }
        }
    };

    setVerticalBoundary(true, settings_.outflowLeft, settings_.dirichletBcLeft);
    setVerticalBoundary(false, settings_.outflowRight, settings_.dirichletBcRight);
}

void Computation::applyObstacleBoundaryValues() {
    // Apply boundary values to the velocity fields caused by obstacles

    for (int i = 0; i < discretization_->nCells()[0]; i++) {
        for (int j = 0; j < discretization_->nCells()[1]; j++) {
            if (discretization_->isObstacleCell(i, j) != 1) {
                continue;
            }

            bool hasLeftNeighbour = discretization_->hasFluidNeighbourLeft(i, j) == 1;
            bool hasRightNeighbour = discretization_->hasFluidNeighbourRight(i, j) == 1;
            bool hasTopNeighbour = discretization_->hasFluidNeighbourTop(i, j) == 1;
            bool hasBottomNeighbour = discretization_->hasFluidNeighbourBottom(i, j) == 1;

            // Set the velocities in normal direction on the edges of the obstacle to zero. Recall
            // the staggered grid in which u of the right neighbour is in cell (i,j)
            if (hasLeftNeighbour) {
                discretization_->u(i - 1, j) = 0;
                discretization_->f(i - 1, j) = 0;

                // Check the surrounding of the obstacle cell and change the velocities in a way,
                // that the interpolations to the obstacle's surface equals 0
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

void Computation::solvePressure() { pressureSolver_->solve(); }

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
