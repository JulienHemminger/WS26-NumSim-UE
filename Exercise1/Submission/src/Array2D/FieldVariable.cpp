#include "Array2D/FieldVariable.h"

FieldVariable::FieldVariable(std::array<int, 2> size,
                             std::array<double, 2> origin,
                             std::array<double, 2> meshWidth)
    : Array2D(size), origin_(origin), meshWidth_(meshWidth) {
    // Todo
}

double FieldVariable::interpolateAt(double x, double y) const {
    // Todo
    return 0.0;
}