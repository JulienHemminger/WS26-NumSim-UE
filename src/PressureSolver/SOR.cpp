#include "SOR.h"

SOR::SOR(std::shared_ptr<Discretization> discretization, double epsilon,
         int maximumNumberOfIterations, double omega)
    : PressureSolver(discretization, epsilon, maximumNumberOfIterations), omega_(omega) {}

void SOR::solve() {
    double dy = discretization_->dy();
    double dx = discretization_->dx();
    double dx2 = dx * dx;
    double dy2 = dy * dy;
    double inv_dx2 = 1.0 / dx2;
    double inv_dy2 = 1.0 / dy2;
    double factor = (dx2 * dy2) / (2.0 * (dx2 + dy2));
    double target_res = epsilon_ * epsilon_;

    int iter = 0;
    double res = calculateResidual();

    while (iter < maximumNumberOfIterations_ && res > target_res) {
        double squaredResidualSum = 0.0;
        int cellCount = 0;

        for (int i = discretization_->pIBegin() + 1; i < discretization_->pIEnd() - 1; i++) {
            for (int j = discretization_->pJBegin() + 1; j < discretization_->pJEnd() - 1; j++) {
                double p_old = discretization_->p(i, j);
                double sum_x =
                    (discretization_->p(i + 1, j) + discretization_->p(i - 1, j)) * inv_dx2;
                double sum_y =
                    (discretization_->p(i, j + 1) + discretization_->p(i, j - 1)) * inv_dy2;

                double p_new = (1.0 - omega_) * p_old +
                               omega_ * factor * (sum_x + sum_y - discretization_->rhs(i, j));
                discretization_->p(i, j) = p_new;

                if (iter % 10 == 0) {
                    double pxx = (discretization_->p(i - 1, j) - 2.0 * p_new +
                                  discretization_->p(i + 1, j)) *
                                 inv_dx2;
                    double pyy = (discretization_->p(i, j - 1) - 2.0 * p_new +
                                  discretization_->p(i, j + 1)) *
                                 inv_dy2;
                    double residual = discretization_->rhs(i, j) - pxx - pyy;
                    squaredResidualSum += residual * residual;
                    cellCount++;
                }
            }
        }

        iter++;
        setBoundaryValues();

        if (iter % 10 == 0) {
            res = squaredResidualSum / cellCount;
        }
    }

    std::cout << "SOR: " << iter << " with a residuum of " << res << " from target " << target_res
              << std::endl;
}