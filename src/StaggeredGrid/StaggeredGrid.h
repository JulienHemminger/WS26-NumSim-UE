#pragma once

#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#include "Array2D/FieldVariable.h"

class StaggeredGrid {
   public:
    // Constructor
    StaggeredGrid(std::array<int, 2> nCells, std::array<double, 2> meshWidth);
    //! get the mesh width, i.e. the length of a single cell in x and y direction
    const std::array<double, 2> meshWidth() const;
    //! get number of cells in each coordinate direction
    const std::array<int, 2> nCells() const;

    //! set obstacle flags
    void setObstacleFlags(std::string pathToGeometry);
    //! set flags of obstacle neighbours
    void setObstacleNeighbourFlags();

    // get a reference to field variable u
    const FieldVariable& u() const;
    // get a reference to field variable v
    const FieldVariable& v() const;
    // get a reference to field variable p
    const FieldVariable& p() const;

    //! access value of u in element (i,j)
    //! get const value of u in element (x,y)
    double u(int i, int j) const;
    //! access value of u in element (x,y)
    double& u(int i, int j);
    //! access value of v in element (i,j)
    //! get const value of v in element (x,y)
    double v(int i, int j) const;
    //! access value of v in element (x,y)
    double& v(int i, int j);
    //! access value of p in element (i,j)
    //! get const value of p in element (x,y)
    double p(int i, int j) const;
    //! access value of p in element (x,y)
    double& p(int i, int j);

    //! acces value of rhs in element (i,j)
    //! access value of rhs in element (x,y)
    double& rhs(int i, int j);
    //! access value of F in element (i,j)
    //! access value of F in element (x,y)
    double& f(int i, int j);
    //! access value of G in element (i,j)
    //! access value of G in element (x,y)
    double& g(int i, int j);

    //! value if cell is an obstacle cell: 1 if obstacle, 0 else
    const FieldVariable& isObstacleCell() const;
    //! access value in element (i,j), get const value in element (x,y)
    double isObstacleCell(int i, int j) const;
    //! access value in element (x,y)
    double& isObstacleCell(int i, int j);

    //! value if obstacle(!) cell if left neighbour is fluid: 1 if true, else 0. Only definded for
    //! obstacles
    const FieldVariable& hasFluidNeighbourLeft() const;
    //! access value in element (i,j), get const value in element (x,y)
    double hasFluidNeighbourLeft(int i, int j) const;
    //! access value in element (x,y)
    double& hasFluidNeighbourLeft(int i, int j);

    //! value if obstacle(!) cell if right neighbour is fluid: 1 if true, else 0. Only definded for
    //! obstacles
    const FieldVariable& hasFluidNeighbourRight() const;
    //! access value in element (i,j), get const value in element (x,y)
    double hasFluidNeighbourRight(int i, int j) const;
    //! access value in element (x,y)
    double& hasFluidNeighbourRight(int i, int j);

    //! value if obstacle(!) cell if top neighbour is fluid: 1 if true, else 0. Only definded for
    //! obstacles
    const FieldVariable& hasFluidNeighbourTop() const;
    //! access value in element (i,j), get const value in element (x,y)
    double hasFluidNeighbourTop(int i, int j) const;
    //! access value in element (x,y)
    double& hasFluidNeighbourTop(int i, int j);

    //! value if obstacle(!) cell if bottom neighbour is fluid: 1 if true, else 0. Only definded for
    //! obstacles
    const FieldVariable& hasFluidNeighbourBottom() const;
    //! access value in element (i,j), get const value in element (x,y)
    double hasFluidNeighbourBottom(int i, int j) const;
    //! access value in element (x,y)
    double& hasFluidNeighbourBottom(int i, int j);

    //! get the mesh width in x-direction
    double dx() const;
    //! get the mesh width in y-direction
    double dy() const;

    //! first valid index for u in x direction
    int uIBegin() const;
    //! one after last valid index for u in x direction
    int uIEnd() const;
    //! first valid index for u in y direction
    int uJBegin() const;
    //! one after last valid index for u in y direction
    int uJEnd() const;

    //! first valid index for v in x direction
    int vIBegin() const;
    //! one after last valid index for v in x direction
    int vIEnd() const;
    //! first valid index for v in y direction
    int vJBegin() const;
    //! one after last valid index for v in y direction
    int vJEnd() const;

    //! first valid index for p in x direction
    int pIBegin() const;
    //! one after last valid index for p in x direction
    int pIEnd() const;
    //! first valid index for p in y direction
    int pJBegin() const;
    //! one after last valid index for p in y direction
    int pJEnd() const;

   protected:
    const std::array<int, 2> nCells_;
    const std::array<double, 2> meshWidth_;
    FieldVariable u_;
    FieldVariable v_;
    FieldVariable p_;
    FieldVariable rhs_;
    FieldVariable f_;
    FieldVariable g_;

    //! obstacle flags
    FieldVariable isObstacleCell_;

    FieldVariable hasFluidNeighbourLeft_;
    FieldVariable hasFluidNeighbourRight_;
    FieldVariable hasFluidNeighbourTop_;
    FieldVariable hasFluidNeighbourBottom_;
};
