#pragma once

#include "PressureSolver/PressureSolver.h"

/**Standard Gauss-Seidel solver.
 */
class GaussSeidel : public PressureSolver{
    public:
        // use constructor of base class
        using PressureSolver::PressureSolver;

        // solve the system of the Poisson equation for pressure
        void solve() override;
}