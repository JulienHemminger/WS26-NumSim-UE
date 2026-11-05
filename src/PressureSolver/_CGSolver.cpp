#include "_CGSolver.h"

CGSolver::CGSolver(std::shared_ptr<Discretization> discretization, double epsilon,
                   int maximumNumberOfIterations, bool outflowBottom, bool outflowTop,
                   bool outflowLeft, bool outflowRight)
    : PressureSolver(discretization, epsilon, maximumNumberOfIterations, outflowBottom, outflowTop,
                     outflowLeft, outflowRight),
      res({discretization_->nCells()[0], discretization_->nCells()[1]},
          {discretization_->meshWidth()[0] / 2., discretization_->meshWidth()[0] / 2},
          discretization_->meshWidth()) {}

void CGSolver::solve() {
    int n = discretization_->nCells()[0];
    int m = discretization_->nCells()[1];
    double alpha = 0;
    double beta = 0;
    double res_pow2_sum = 0;
    double res_pow2_sum_updated = 0;
    int iteration = 0;

    FieldVariable res_pow2(
        {n, m}, {discretization_->meshWidth()[0] / 2., discretization_->meshWidth()[0] / 2},
        discretization_->meshWidth());
    FieldVariable c({n, m},
                    {discretization_->meshWidth()[0] / 2., discretization_->meshWidth()[0] / 2},
                    discretization_->meshWidth());
    FieldVariable Ac({n, m},
                     {discretization_->meshWidth()[0] / 2., discretization_->meshWidth()[0] / 2},
                     discretization_->meshWidth());

    double dy = discretization_->dy();
    double dx = discretization_->dx();
    double dx2 = pow(dx, 2);
    double dy2 = pow(dy, 2);

    int nFluidCells = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (discretization_->isObstacleCell(i, j) != 1.) {
                nFluidCells++;
            }
        }
    }

    calculateInitialResidual();

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (discretization_->isObstacleCell(i, j) != 1.) {
                res_pow2(i, j) = res(i, j) * res(i, j);
                res_pow2_sum += res_pow2(i, j);
            }
        }
    }

    while (iteration < maximumNumberOfIterations_) {
        double cAc = 0;

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                if (discretization_->isObstacleCell(i, j) != 1.) {
                    double pxx = (discretization_->p(i - 1, j) - 2.0 * discretization_->p(i, j) +
                                  discretization_->p(i + 1, j)) /
                                 (dx2);
                    double pyy = (discretization_->p(i, j - 1) - 2.0 * discretization_->p(i, j) +
                                  discretization_->p(i, j + 1)) /
                                 (dy2);

                    Ac(i, j) = pxx + pyy;
                    cAc += c(i, j) * Ac(i, j);
                }
            }
        }

        alpha = res_pow2_sum / cAc;

        res_pow2_sum_updated = 0;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                if (discretization_->isObstacleCell(i, j) != 1.) {
                    discretization_->p(i, j) = discretization_->p(i, j) + alpha * c(i, j);
                    res(i, j) = res(i, j) - alpha * Ac(i, j);
                    res_pow2_sum_updated += res(i, j) * res(i, j);
                }
            }
        }

        if ((res_pow2_sum / nFluidCells) <= pow(epsilon_, 2)) {
            setBoundaryValues();
            break;
        }

        beta = res_pow2_sum_updated / res_pow2_sum;

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                if (discretization_->isObstacleCell(i, j) != 1.) {
                    c(i, j) = res(i, j) + beta * c(i, j);
                }
            }
        }

        res_pow2_sum = res_pow2_sum_updated;
        iteration++;
    }

    std::cout << "CG: " << iteration << " with a residuum of " << res_pow2_sum_updated
              << " from target " << epsilon_ << std::endl;
}

void CGSolver::calculateInitialResidual() {
    int nCellsx = discretization_->nCells()[0];
    int nCellsy = discretization_->nCells()[1];

    double dy = discretization_->dy();
    double dx = discretization_->dx();
    double dx2 = pow(dx, 2);
    double dy2 = pow(dy, 2);

    for (int i = discretization_->pIBegin() + 1; i < discretization_->pIEnd() - 1; i++) {
        for (int j = discretization_->pJBegin() + 1; j < discretization_->pJEnd() - 1; j++) {
            if (discretization_->isObstacleCell(i, j) != 1.) {
                double pxx = (discretization_->p(i - 1, j) - 2.0 * discretization_->p(i, j) +
                              discretization_->p(i + 1, j)) /
                             (dx2);
                double pyy = (discretization_->p(i, j - 1) - 2.0 * discretization_->p(i, j) +
                              discretization_->p(i, j + 1)) /
                             (dy2);

                res(i, j) = discretization_->rhs(i, j) - pxx - pyy;
            }
        }
    }
}
