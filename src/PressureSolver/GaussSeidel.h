#pragma once

#include "PressureSolver/PressureSolver.h"

class GaussSeidel : public PressureSolver {
   public:
    using PressureSolver::PressureSolver;

    void solve();
};