#pragma once

#include <memory>

#include "StaggeredGrid/Discretization.h"

/** Interface for the pressure solver. It computes the pressure field variable such that the
 * continuity equation is fulfilled.
 */
class PressureSolver {
   public:
    // constructor
    PressureSolver(std::shared_ptr<Discretization> discretization, double epsilon,
                   int maximumNumberOfIterations, bool outflowBottom, bool outflowTop,
                   bool outflowLeft, bool outflowRight);
    // solve the Poisson problem for the pressure, using the rhs and p field variables in the
    // staggeredGrid
    virtual void solve() = 0;

   protected:
    // set the boundary values to account for homogenous Neumann boundary conditions, this has to be
    // called after every iteration
    void setBoundaryValues();
    void setBoundaryValuesLeft();
    void setBoundaryValuesRight();
    void setBoundaryValuesTop();
    void setBoundaryValuesBottom();

    void applyObstacleBoundaryValues();

    double calculateResidual();

   protected:
    std::shared_ptr<Discretization> discretization_;  // object holding the needed field variables for rhs and p
    double epsilon_;
    int maximumNumberOfIterations_;
    bool outflowBottom_;
    bool outflowTop_;
    bool outflowLeft_;
    bool outflowRight_;
};