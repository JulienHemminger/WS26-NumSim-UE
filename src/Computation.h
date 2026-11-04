#pragma once

#include <array>
#include <memory>

#include "OutputWriter/OutputWriterParaview.h"
#include "OutputWriter/OutputWriterText.h"
#include "PressureSolver/PressureSolver.h"
#include "Settings.h"
#include "StaggeredGrid/Discretization.h"

/**This class handles the main simulation. It implements the time stepping scheme, computes all the
 * terms and calls the pressure solver.
 */

class Computation {
   public:
    void initialize(int argc, char* argv[]);

    // run the whole simulation until tend
    void runSimulation();

   private:
    // compute the time step width dt from maximum velocities
    void computeTimeStepWidth();

    // set boundary values of u and v to correct values
    void applyBoundaryValues();

    // compute the preliminary velocities, F and G
    void computePreliminaryVelocities();

    // compute the right hand side of the Poisson equation for the pressure
    void computeRightHandSide();

    // solve the Poisson equation for the pressure
    void solvePressure();

    // compute the new velocities, u,v, from the preliminary velocities, F,G and the pressure, p
    void computeVelocities();

   private:
    Settings settings_;
    std::shared_ptr<Discretization> discretization_;
    std::unique_ptr<PressureSolver> pressureSolver_;
    std::unique_ptr<OutputWriterParaview> outputWriterParaview_;
    std::unique_ptr<OutputWriterText> outputWriterText_;
    std::array<double, 2> meshWidth_;
    double dt_;
};