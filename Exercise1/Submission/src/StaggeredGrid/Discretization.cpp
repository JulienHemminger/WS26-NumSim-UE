#include "StaggeredGrid/Discretization.h"

Discretization::Discretization(std::array<int, 2> nCells,
                               std::array<double, 2> meshWidth)
    : StaggeredGrid(nCells, meshWidth) {
    // Todo
}

double Discretization::computeD2uDx2(int i, int j) const {
    // Todo
    return 0.0;
}

double Discretization::computeD2uDy2(int i, int j) const {
    // Todo
    return 0.0;
}

double Discretization::computeD2vDx2(int i, int j) const {
    // Todo
    return 0.0;
}

double Discretization::computeD2vDy2(int i, int j) const {
    // Todo
    return 0.0;
}

double Discretization::computeDpDx(int i, int j) const {
    // Todo
    return 0.0;
}

double Discretization::computeDpDy(int i, int j) const {
    // Todo
    return 0.0;
}
