#include "StaggeredGrid/DonorCell.h"

DonorCell::DonorCell(std::array<int, 2> nCells,
                     std::array<double, 2> meshWidth,
                     double alpha)
    : Discretization(nCells, meshWidth), alpha_(alpha) {
    // Todo
}

double DonorCell::computeDu2Dx(int i, int j) const {
    // Todo
    return 0.0;
}

double DonorCell::computeDv2Dy(int i, int j) const {
    // Todo
    return 0.0;
}

double DonorCell::computeDuvDx(int i, int j) const {
    // Todo
    return 0.0;
}

double DonorCell::computeDuvDy(int i, int j) const {
    // Todo
    return 0.0;
}