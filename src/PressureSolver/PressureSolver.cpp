#include "PressureSolver/PressureSolver.h"

PressureSolver::PressureSolver(std::shared_ptr<Discretization> discretization,
                               double epsilon,
                               int maximumNumberOfIterations)
    : discretization_(discretization),
      epsilon_(epsilon),
      maximumNumberOfIterations_(maximumNumberOfIterations) {
    // Todo
}

void PressureSolver::setBoundaryValues() {
    // Todo
}