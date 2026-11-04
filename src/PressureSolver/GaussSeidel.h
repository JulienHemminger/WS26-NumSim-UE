#pragma once

#include "PressureSolver/PressureSolver.h"

/**Standard Gauss-Seidel solver.
 *
 * Implementing the Gauss Seidel scheme is voluntary. See
 * https://numsim-exercises.readthedocs.io/en/latest/exercise1/submission.html
 */
class GaussSeidel : public PressureSolver {
   public:
    // use constructor of base class
    using PressureSolver::PressureSolver;

    // solve the system of the Poisson equation for pressure
    void solve() override;
};