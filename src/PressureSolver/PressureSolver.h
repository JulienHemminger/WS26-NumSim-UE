#pragma once

#include <memory>

#include "StaggeredGrid/Discretization.h"

class PressureSolver {
   public:
    PressureSolver(std::shared_ptr<Discretization> discretization, double epsilon,
                   int maximumNumberOfIterations, bool outflowBottom, bool outflowTop,
                   bool outflowLeft, bool outflowRight);
    virtual void solve() = 0;

   protected:
    void setBoundaryValues();
    void setBoundaryValuesLeft();
    void setBoundaryValuesRight();
    void setBoundaryValuesTop();
    void setBoundaryValuesBottom();

    void applyObstacleBoundaryValues();

    double calculateResidual();

   protected:
    std::shared_ptr<Discretization> discretization_;
    double epsilon_;
    int maximumNumberOfIterations_;
    bool outflowBottom_;
    bool outflowTop_;
    bool outflowLeft_;
    bool outflowRight_;
};