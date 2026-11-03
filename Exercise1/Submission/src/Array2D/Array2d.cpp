#include "Array2D/Array2D.h"

Array2D::Array2D(std::array<int, 2> size)
    : data_(static_cast<std::size_t>(size[0] * size[1]), 0.0), size_(size) {
    // Todo
}

std::array<int, 2> Array2D::size() const {
    // Todo
    return size_;
}

double& Array2D::operator()(int i, int j) {
    // Todo
    return data_[static_cast<std::size_t>(i * size_[1] + j)];
}

double Array2D::operator()(int i, int j) const {
    // Todo
    return data_[static_cast<std::size_t>(i * size_[1] + j)];
}