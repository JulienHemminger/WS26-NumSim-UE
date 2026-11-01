# include "PressureSolver/SOR.h"

SOR::SOR(std::shared_ptr<Discretization> discretization, double epsilon, int maximumNumberOfIterations, double omega)
 : PressureSolver(discretization, epsilon, maximumNumberOfIterations), omega_(omega){
}

void SOR::solve(){
}

