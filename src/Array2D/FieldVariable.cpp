#include "Array2D/FieldVariable.h"

FieldVariable::FieldVariable(std::array<int, 2> size, std::array<double, 2> origin,
                             std::array<double, 2> meshWidth)
    : Array2D(size), origin_(origin), meshWidth_(meshWidth){};

double FieldVariable::interpolateAt(double x, double y) const {
    const double dx = meshWidth_[0];
    const double dy = meshWidth_[1];

    int iLeftEdge = (int)std::floor((x + origin_[0]) / dx);
    int jLowerEdge = (int)std::floor((y + origin_[1]) / dy);

    if (iLeftEdge >= size_[0] - 1) {
        --iLeftEdge;
    }
    if (jLowerEdge >= (*this).size_[1] - 1) {
        --jLowerEdge;
    }

    const double xr1 = x - ((meshWidth_[0] * iLeftEdge) - origin_[0]);
    const double yr1 = y - ((meshWidth_[1] * jLowerEdge) - origin_[1]);
    const double xr2 = dx - xr1;
    const double yr2 = dy - yr1;

    int xLeftEdge = iLeftEdge;
    int yLowerEdge = jLowerEdge;

    const double f_lowerLeft = (*this)(xLeftEdge, yLowerEdge);
    const double f_upperLeft = (*this)(xLeftEdge, yLowerEdge + 1);
    const double f_lowerRight = (*this)(xLeftEdge + 1, yLowerEdge);
    const double f_upperRight = (*this)(xLeftEdge + 1, yLowerEdge + 1);

    const double f_intp = (f_lowerLeft * xr2 * yr2 + f_lowerRight * xr1 * yr2 +
                           f_upperLeft * xr2 * yr1 + f_upperRight * xr1 * yr1) /
                          (dx * dy);

    return f_intp;
}
