#pragma once

#include "PressureSolver/PressureSolver.h"

/** Successive over-relaxation solver.
*/
class SOR : public PressureSolver{
    public:
        // constructor
        SOR(std::shared_ptr<Discretization> discretization, double epsilon, int maximumNumberOfIterations, double omega);

        // solve the system of the Poisson equation for pressure
        void solve() override;

    private:
        double omega_;
};