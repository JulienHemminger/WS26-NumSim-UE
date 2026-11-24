#pragma once

#include <math.h>
#include <mpi.h>

#include <cmath>
#include <iostream>
#include <vector>

#include "Partitioning/Partitioning.h"
#include "PressureSolver/PressureSolver.h"

/**
 * https://numsim-exercises.readthedocs.io/en/latest/exercise2/hints.html#pressure-solver
 *
 */
class RedBlack : public PressureSolver {
   public:
    //! use constructor of parent class pressure solver
    RedBlack(std::shared_ptr<Discretization> discretization, double epsilon,
             int maximumNumberOfIterations, std::shared_ptr<Partitioning> partitioning);

    //! solve the pressure poisson equation
    void solve();

    //! calculate residual of pressure equation
    double calculateResidual();

    //! sets the ghost layers in the discretization correctly
    void pExchangeVertical();
    void pExchangeHorizontal();

    //! helper that exchanges the ghost layers for p
    void exchange(int rankCorrespondent,  //! rank of the other process with which to exchange
                  int indexToSend,        //! index of the slice to send
                  int indexFromReceive,   //! index of the slice where to write the incoming data
                  char direction,         //! x or y direction
                  bool ToFrom);           //! send and then receive, else receive then send

   protected:
    std::shared_ptr<Partitioning> partitioning_;  //! partitioning object as shared pointer
};
