#pragma once

#include <cmath>
#include <vector>

#include "PressureSolver/PressureSolver.h"

class CGSolver : public PressureSolver {
   public:
    CGSolver(std::shared_ptr<Discretization> discretization, double epsilon,
             int maximumNumberOfIterations, bool outflowBottom, bool outflowTop, bool outflowLeft,
             bool outflowRight);

    void solve();
    void calculateInitialResidual();

   private:
    FieldVariable res;
};