#pragma once

#include <cmath>

#include "PressureSolver/PressureSolver.h"

/** Successive over-relaxation solver.
 */
class SOR : public PressureSolver {
   public:
    // constructor
    SOR(std::shared_ptr<Discretization> discretization, double epsilon,
        int maximumNumberOfIterations, double omega, bool outflowBottom, bool outflowTop,
        bool outflowLeft, bool outflowRight);

    // solve the system of the Poisson equation for pressure
    void solve();

   private:
    double omega_;
};
