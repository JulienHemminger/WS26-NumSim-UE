#pragma once

#include "Settings.h"
#include "StaggeredGrid/CentralDifferences.h"
#include "StaggeredGrid/DonorCell.h"
#include "PressureSolver/SOR.h"
#include "PressureSolver/GaussSeidel.h"
#include "PressureSolver/_CGSolver.h"
#include "OutputWriter/OutputWriterParaview.h"
#include "OutputWriter/OutputWriterText.h"
#include "Array2D/FieldVariable.h"

#include <memory>
#include <iostream>  // for cout
#include <cmath>
#include <algorithm>
#include <cmath>

class Computation
{
public:    
    //! initialize the computation object, parse the settings from file that is given as the only command line argument
    void initialize(int argc, char *argv[]);

    //! run the whole simulation until t_end 
    void runSimulation();

    //! test the simulation, especially the pressure solver
    void runTest();

private:
    //! compute the time step width dt from maximum velocities 
    void computeTimeStepWidth(double currentTime);

    //!  set boundary values of u , v, F, G to correct values
    void applyBoundaryValues();

    //!  set boundary values of u, v, F, G around obstacle cells
    void applyObstacleBoundaryValues();

    //! compute the preliminary velocities, F and G
    void computePreliminaryVelocities();

    //! compute the right hand side of the Poisson equation for the pressure
    void computeRightHandSide();

    //! solve the Poisson equation for the pressure 
    void computePressure();

    //! compute the new velocities, u,v, from the preliminary velocities, F,G and the pressure, p
    void computeVelocities();

    Settings settings_;

    std::shared_ptr<Discretization> discretization_;
    std::unique_ptr<PressureSolver> pressureSolver_;

    std::unique_ptr<OutputWriterParaview> outputWriterParaview_;
    std::unique_ptr<OutputWriterText> outputWriterText_;

    std::array<double, 2> meshWidth_; //< h_x, h_y
    double dt_; //< time step
};
