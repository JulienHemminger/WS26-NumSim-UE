#include "StaggeredGrid/StaggeredGrid.h"

StaggeredGrid::StaggeredGrid(std::array<int, 2> nCells,
                             std::array<double, 2> meshWidth)
    : nCells_(nCells),
      meshWidth_(meshWidth),
      u_(nCells, {0.0, 0.0}, meshWidth),
      v_(nCells, {0.0, 0.0}, meshWidth),
      p_(nCells, {0.0, 0.0}, meshWidth),
      rhs_(nCells, {0.0, 0.0}, meshWidth),
      f_(nCells, {0.0, 0.0}, meshWidth),
      g_(nCells, {0.0, 0.0}, meshWidth) {
    // Todo
}

const std::array<double, 2> StaggeredGrid::meshWidth() {
    // Todo
    return meshWidth_;
}

const std::array<int, 2> StaggeredGrid::nCells() {
    // Todo
    return nCells_;
}

const FieldVariable& StaggeredGrid::u() {
    // Todo
    return u_;
}

const FieldVariable& StaggeredGrid::v() {
    // Todo
    return v_;
}

const FieldVariable& StaggeredGrid::p() {
    // Todo
    return p_;
}

double StaggeredGrid::u(int i, int j) const {
    // Todo
    return u_(i, j);
}

double& StaggeredGrid::u(int i, int j) {
    // Todo
    return u_(i, j);
}

double StaggeredGrid::v(int i, int j) const {
    // Todo
    return v_(i, j);
}

double& StaggeredGrid::v(int i, int j) {
    // Todo
    return v_(i, j);
}

double StaggeredGrid::p(int i, int j) const {
    // Todo
    return p_(i, j);
}

double& StaggeredGrid::p(int i, int j) {
    // Todo
    return p_(i, j);
}

double& StaggeredGrid::rhs(int i, int j) {
    // Todo
    return rhs_(i, j);
}

double& StaggeredGrid::f(int i, int j) {
    // Todo
    return f_(i, j);
}

double& StaggeredGrid::g(int i, int j) {
    // Todo
    return g_(i, j);
}

double StaggeredGrid::dx() const {
    // Todo
    return meshWidth_[0];
}

double StaggeredGrid::dy() const {
    // Todo
    return meshWidth_[1];
}

int StaggeredGrid::uIBegin() const {
    // Todo
    return 0;
}

int StaggeredGrid::uIEnd() const {
    // Todo
    return nCells_[0];
}

int StaggeredGrid::uJBegin() const {
    // Todo
    return 0;
}

int StaggeredGrid::uJEnd() const {
    // Todo
    return nCells_[1];
}

int StaggeredGrid::vIBegin() const {
    // Todo
    return 0;
}

int StaggeredGrid::vIEnd() const {
    // Todo
    return nCells_[0];
}

int StaggeredGrid::vJBegin() const {
    // Todo
    return 0;
}

int StaggeredGrid::vJEnd() const {
    // Todo
    return nCells_[1];
}

int StaggeredGrid::pIBegin() const {
    // Todo
    return 0;
}

int StaggeredGrid::pIEnd() const {
    // Todo
    return nCells_[0];
}

int StaggeredGrid::pJBegin() const {
    // Todo
    return 0;
}

int StaggeredGrid::pJEnd() const {
    // Todo
    return nCells_[1];
}