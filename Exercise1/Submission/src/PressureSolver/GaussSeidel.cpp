#include "PressureSolver/GaussSeidel.h"

GaussSeidel::GaussSeidel(std::shared_ptr<Discretization> discretization, double epsilon, int maximumNumberOfIterations)
 : PressureSolver(discretization, epsilon, maximumNumberOfIterations){
}

void GaussSeidel::solve(){
}