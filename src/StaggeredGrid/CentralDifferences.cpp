#include "StaggeredGrid/CentralDifferences.h"

//! compute the 1st derivative ∂ u^2 / ∂x
double CentralDifferences::computeDu2Dx(int i, int j) const {
    const double uCenterRight = (u(i + 1, j) + u(i, j)) / 2.;
    const double uCenterCell = (u(i, j) + u(i - 1, j)) / 2.;

    return (uCenterRight * uCenterRight - uCenterCell * uCenterCell) / dx();
}

//! compute the 1st derivative ∂ v^2 / ∂x
double CentralDifferences::computeDv2Dy(int i, int j) const {
    const double vCenterAbove = (v(i, j + 1) + v(i, j)) / 2.0;
    const double vCenterCell = (v(i, j) + v(i, j - 1)) / 2.0;

    return (vCenterAbove * vCenterAbove - vCenterCell * vCenterCell) / dy();
}

//! compute the 1st derivative ∂ (uv) / ∂x at top edge of cell (for v)
double CentralDifferences::computeDuvDx(int i, int j) const {
    const double uTopRight = (u(i, j + 1) + u(i, j)) / 2.0;
    const double uTopLeft = (u(i - 1, j + 1) + u(i - 1, j)) / 2.0;

    const double vTopRight = (v(i + 1, j) + v(i, j)) / 2.0;
    const double vTopLeft = (v(i, j) + v(i - 1, j)) / 2.0;

    return (uTopRight * vTopRight - uTopLeft * vTopLeft) / dx();
}

//! compute the 1st derivative ∂ (uv) / ∂y at right edge of cell (for u)
double CentralDifferences::computeDuvDy(int i, int j) const {
    const double uTopRight = (u(i, j + 1) + u(i, j)) / 2.0;
    const double uBottomRight = (u(i, j) + u(i, j - 1)) / 2.0;

    const double vTopRight = (v(i + 1, j) + v(i, j)) / 2.0;
    const double vBottomRight = (v(i + 1, j - 1) + v(i, j - 1)) / 2.0;

    return (uTopRight * vTopRight - uBottomRight * vBottomRight) / dy();
}