#include "GaussSeidel.h"

#include <cmath>

void GaussSeidel::solve() {
    const double dy = discretization_->dy();
    const double dx = discretization_->dx();
    const double dx2 = dx * dx;
    const double dy2 = dy * dy;

    int iteration = 0;

    double res = calculateResidual();

    while (iteration < maximumNumberOfIterations_ && res > pow(epsilon_, 2)) {
        for (int i = discretization_->pIBegin() + 1; i < discretization_->pIEnd() - 1; i++) {
            for (int j = discretization_->pJBegin() + 1; j < discretization_->pJEnd() - 1; j++) {
                double sum_x =
                    (discretization_->p(i + 1, j) + discretization_->p(i - 1, j)) / (dx2);
                double sum_y =
                    (discretization_->p(i, j + 1) + discretization_->p(i, j - 1)) / (dy2);
                discretization_->p(i, j) =
                    (dx2 * dy2) / (2 * (dx2 + dy2)) * (sum_x + sum_y - discretization_->rhs(i, j));
            }
        }

        iteration += 1;

        setBoundaryValues();

        res = calculateResidual();
    }

    std::cout << "Gauss-Seidel: " << iteration << " with a residuum of " << res << " from target "
              << epsilon_ << std::endl;
}
