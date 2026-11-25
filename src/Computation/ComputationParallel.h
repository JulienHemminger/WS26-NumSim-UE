#pragma once

#include <mpi.h>

#include <algorithm>
#include <cmath>
#include <iostream>  // for cout
#include <memory>
#include <vector>

#include "Computation.h"
#include "OutputWriter/OutputWriterParaviewParallel.h"
#include "OutputWriter/OutputWriterTextParallel.h"
#include "Partitioning/Partitioning.h"
#include "PressureSolver/RedBlack.h"
#include "StaggeredGrid/Discretization.h"

class ComputationParallel : public Computation {
   public:
    //! initialize the computation object, parse the settings from file that is given as the only
    //! command line argument
    void initialize(int argc, char* argv[]);  // overwrite

    //! run the whole simulation until t_end
    void runSimulation();  // overwrite

   protected:
    //! compute the time step width dt from maximum velocities
    void computeTimeStepWidthParallel(double currentTime);

    //!  set boundary values of u and v to correct values
    void applyBoundaryValues();  // overwrite of method by Computation
    void applyBoundaryValuesLeft();
    void applyBoundaryValuesRight();
    void applyBoundaryValuesTop();
    void applyBoundaryValuesBottom();

    void exchangeVelocityHalosHorizontal();
    void exchangeVelocityHalosVertical();
    void exchangePreliminaryVelocityHalosHorizontal();
    void exchangePreliminaryVelocityHalosVertical();

    //! sets the ghost layers in the discretization correctly
    void uvExchangeVertical();
    void uvExchangeHorizontal();

    //! exchanges the ghost layers for u, v
    std::shared_ptr<Partitioning> partitioning_;

    std::unique_ptr<OutputWriterParaviewParallel> outputWriterParaviewParallel_;
    std::unique_ptr<OutputWriterTextParallel> outputWriterTextParallel_;
};
