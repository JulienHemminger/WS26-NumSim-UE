#include "PressureSolver.h"

#include <math.h>

#include <memory>

PressureSolver::PressureSolver(std::shared_ptr<Discretization> discretization, double epsilon,
                               int maximumNumberOfIterations, bool outflowBottom, bool outflowTop,
                               bool outflowLeft, bool outflowRight)
    : discretization_(discretization),
      epsilon_(epsilon),
      maximumNumberOfIterations_(maximumNumberOfIterations),
      outflowBottom_(outflowBottom),
      outflowTop_(outflowTop),
      outflowLeft_(outflowLeft),
      outflowRight_(outflowRight) {}

void PressureSolver::setBoundaryValues() {
    setBoundaryValuesTop();
    setBoundaryValuesBottom();

    setBoundaryValuesLeft();
    setBoundaryValuesRight();

    applyObstacleBoundaryValues();
}

void PressureSolver::applyObstacleBoundaryValues() {
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
                if (hasTopNeighbour) {
                    discretization_->p(i, j) =
                        0.5 * (discretization_->p(i - 1, j) + discretization_->p(i, j + 1));
                } else if (hasBottomNeighbour) {
                    discretization_->p(i, j) =
                        0.5 * (discretization_->p(i - 1, j) + discretization_->p(i, j - 1));
                } else {
                    discretization_->p(i, j) = discretization_->p(i - 1, j);
                }
            }

            else if (hasRightNeighbour) {
                if (hasTopNeighbour) {
                    discretization_->p(i, j) =
                        0.5 * (discretization_->p(i, j + 1) + discretization_->p(i + 1, j));
                } else if (hasBottomNeighbour) {
                    discretization_->p(i, j) =
                        0.5 * (discretization_->p(i + 1, j) + discretization_->p(i, j - 1));
                } else {
                    discretization_->p(i, j) = discretization_->p(i + 1, j);
                }
            }

            else if (hasTopNeighbour) {
                discretization_->p(i, j) = discretization_->p(i, j + 1);
            }

            else if (hasBottomNeighbour) {
                discretization_->p(i, j) = discretization_->p(i, j - 1);
            }
        }
    }
}

void PressureSolver::setBoundaryValuesLeft() {
    for (int j = discretization_->pJBegin(); j < discretization_->pJEnd(); j++) {
        if (outflowLeft_) {
            discretization_->p(-1, j) = -discretization_->p(0, j);
        } else {
            discretization_->p(-1, j) = discretization_->p(0, j);
        }
    }
}

void PressureSolver::setBoundaryValuesRight() {
    int n = discretization_->nCells()[0];

    for (int j = discretization_->pJBegin(); j < discretization_->pJEnd(); j++) {
        if (outflowRight_) {
            discretization_->p(n, j) = -discretization_->p(n - 1, j);
        } else {
            discretization_->p(n, j) = discretization_->p(n - 1, j);
        }
    }
}

void PressureSolver::setBoundaryValuesTop() {
    int m = discretization_->nCells()[1];

    for (int i = discretization_->pIBegin() + 1; i < discretization_->pIEnd() - 1; i++) {
        if (outflowTop_) {
            discretization_->p(i, m) = -discretization_->p(i, m - 1);
        } else {
            discretization_->p(i, m) = discretization_->p(i, m - 1);
        }
    }
}

void PressureSolver::setBoundaryValuesBottom() {
    for (int i = discretization_->pIBegin() + 1; i < discretization_->pIEnd() - 1; i++) {
        if (outflowBottom_) {
            discretization_->p(i, -1) = -discretization_->p(i, 0);
        } else {
            discretization_->p(i, -1) = discretization_->p(i, 0);
        }
    }
}

double PressureSolver::calculateResidual() {
    const double dx = discretization_->dx();
    const double dy = discretization_->dy();
    const double dx2 = pow(dx, 2);
    const double dy2 = pow(dy, 2);

    double squaredResidualSum = 0.0;
    int fluidCellCount = 0;

    for (int i = discretization_->pIBegin() + 1; i < discretization_->pIEnd() - 1; i++) {
        for (int j = discretization_->pJBegin() + 1; j < discretization_->pJEnd() - 1; j++) {
            if (discretization_->isObstacleCell(i, j) != 1.) {
                const double pxx = (discretization_->p(i - 1, j) - 2.0 * discretization_->p(i, j) +
                                    discretization_->p(i + 1, j)) /
                                   dx2;
                const double pyy = (discretization_->p(i, j - 1) - 2.0 * discretization_->p(i, j) +
                                    discretization_->p(i, j + 1)) /
                                   dy2;
                const double residual = discretization_->rhs(i, j) - pxx - pyy;

                squaredResidualSum += pow(residual, 2);
                fluidCellCount++;
            }
        }
    }

    return squaredResidualSum / fluidCellCount;
}
