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
            if (discretization_->isObstacleCell(i, j) == 1) {
                if (discretization_->hasFluidNeighbourLeft(i, j) == 1.) {
                    if (discretization_->hasFluidNeighbourTop(i, j) == 1.) {
                        discretization_->p(i, j) =
                            0.5 * (discretization_->p(i - 1, j) + discretization_->p(i, j + 1));
                    } else if (discretization_->hasFluidNeighbourBottom(i, j) == 1) {
                        discretization_->p(i, j) =
                            0.5 * (discretization_->p(i - 1, j) + discretization_->p(i, j - 1));
                    } else {
                        discretization_->p(i, j) = discretization_->p(i - 1, j);
                    }
                }

                else if (discretization_->hasFluidNeighbourRight(i, j) == 1) {
                    if (discretization_->hasFluidNeighbourTop(i, j) == 1) {
                        discretization_->p(i, j) =
                            0.5 * (discretization_->p(i, j + 1) + discretization_->p(i + 1, j));
                    } else if (discretization_->hasFluidNeighbourBottom(i, j) == 1) {
                        discretization_->p(i, j) =
                            0.5 * (discretization_->p(i + 1, j) + discretization_->p(i, j - 1));
                    } else {
                        discretization_->p(i, j) = discretization_->p(i + 1, j);
                    }
                }

                else if (discretization_->hasFluidNeighbourTop(i, j) == 1) {
                    discretization_->p(i, j) = discretization_->p(i, j + 1);
                }

                else if (discretization_->hasFluidNeighbourBottom(i, j) == 1) {
                    discretization_->p(i, j) = discretization_->p(i, j - 1);
                }
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
    int nCellsx = discretization_->nCells()[0];
    int nCellsy = discretization_->nCells()[1];

    double dy = discretization_->dy();
    double dx = discretization_->dx();
    double dx2 = pow(dx, 2);
    double dy2 = pow(dy, 2);

    double res = 0.0;
    int nFluidCells = 0;

    for (int i = discretization_->pIBegin() + 1; i < discretization_->pIEnd() - 1; i++) {
        for (int j = discretization_->pJBegin() + 1; j < discretization_->pJEnd() - 1; j++) {
            if (discretization_->isObstacleCell(i, j) != 1.) {
                double pxx = (discretization_->p(i - 1, j) - 2.0 * discretization_->p(i, j) +
                              discretization_->p(i + 1, j)) /
                             (dx2);
                double pyy = (discretization_->p(i, j - 1) - 2.0 * discretization_->p(i, j) +
                              discretization_->p(i, j + 1)) /
                             (dy2);

                double resij = discretization_->rhs(i, j) - pxx - pyy;
                res = res + (pow(resij, 2));
                nFluidCells++;
            }
        }
    }

    res = res / (nFluidCells);
    return res;
}
