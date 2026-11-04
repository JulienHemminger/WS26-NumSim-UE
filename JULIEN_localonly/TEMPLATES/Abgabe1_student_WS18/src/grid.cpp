#include "grid.hpp"
#include "geometry.hpp"
#include "iterator.hpp"

 /// Constructs a grid based on a geometry
  Grid::Grid(const Geometry *geom):Grid(geom, multi_real_t (0,0)){ // using initializer list as Grid has no default constructor
      
  }

  /// Constructs a grid based on a geometry with an offset
  // @param geom   Geometry information
  // @param offset distance of staggered grid point to cell's anchor point;
  //               (anchor point = lower left corner)
  Grid::Grid(const Geometry *geom, const multi_real_t &offset){
      _geom = geom;
      _offset = offset;
      index_t dim = _geom->Size()[1]* _geom->Size()[0];
      _data = new real_t[dim];
  }

  /// Deletes the grid
  Grid::~Grid(){
      delete [] _data;
  }

  ///     Initializes the grid with a value
  void Grid::Initialize(const real_t &value){
      index_t dim = _geom->Size()[1]* _geom->Size()[0];
      for (index_t i=0; i<dim; ++i){
          _data[i] = value;
      }
  }

  /// Write access to the grid cell at position [it]
  real_t &Grid::Cell(const Iterator &it){
      return _data[it];
  }
  /// Read access to the grid cell at position [it]
  const real_t &Grid::Cell(const Iterator &it) const{
      return _data[it];
  }

  /// Interpolate the value at a arbitrary position
  real_t Grid::Interpolate(const multi_real_t &pos) const{
      // normalize position for easier programming
      real_t pos_x = pos[0] / _geom->Mesh()[0] + 1.0;
      real_t pos_y = pos[1] / _geom->Mesh()[1] + 1.0;

      // clamp grid cell to (1,1) (grid cells here without boundary ghostcells)
      pos_x = std::max(1.0, std::min((real_t)(_geom->Size()[0] - 1), pos_x));
      pos_y = std::max(1.0, std::min((real_t)(_geom->Size()[1] - 1), pos_y));

      // subtract offset of considered grid (all grids have the same vertices now)
      pos_x = pos_x - (_offset[0] / _geom->Mesh()[0]);
      pos_y = pos_y - (_offset[1] / _geom->Mesh()[1]);

      // distance to lower left corner of the grid
      real_t dist_x = pos_x - floor(pos_x);
      real_t dist_y = pos_y - floor(pos_y);

      // determine grid cell index
      index_t cell_no_x = floor(pos_x);
      index_t cell_no_y = floor(pos_y);

      // corresponding Iterator with starting value
      index_t start_value = cell_no_y * _geom->Size()[0] + cell_no_x;
      Iterator it(_geom, start_value);

      // bilinear interpolation
      real_t lin_down = (1-dist_x) * this->Cell(it) + dist_x * this->Cell(it.Right());
      real_t lin_top = (1-dist_x) * this->Cell(it.Top()) + dist_x * this->Cell(it.Right().Top());
      real_t interp_result = (1.0 - dist_y) * lin_down + dist_y * lin_top;

      return interp_result;

  }

  /// Computes the left-sided difference quotient in x-dim at [it]
  real_t Grid::dx_l(const Iterator &it) const{
      real_t dx_l = 1.0/(_geom->Mesh()[0]) * (_data[it] - _data[it.Left()]);
      return dx_l;
  }
  /// Computes the right-sided difference quotient in x-dim at [it]
  real_t Grid::dx_r(const Iterator &it) const{
      real_t dx_r = 1.0/(_geom->Mesh()[0])*(_data[it.Right()] - _data[it]);
      return dx_r;
  }
  /// Computes the left-sided difference quotient in y-dim at [it]
  real_t Grid::dy_l(const Iterator &it) const{
      real_t dy_l = 1.0/(_geom->Mesh()[1])*(_data[it] - _data[it.Down()]);
      return dy_l;
  }
  /// Computes the right-sided difference quotient in x-dim at [it]
  real_t Grid::dy_r(const Iterator &it) const{
      real_t dy_r = 1.0/(_geom->Mesh()[1])*(_data[it.Top()] - _data[it]);
      return dy_r;
  }
  /// Computes the central difference quotient of 2nd order in x-dim at [it]
  real_t Grid::dxx(const Iterator &it) const{
      real_t numerator = _data[it.Right()] - 2.0 * _data[it] + _data[it.Left()];
      real_t dxx = 1.0/(_geom->Mesh()[0]*_geom->Mesh()[0])*numerator;
      return dxx;
  }
  /// Computes the central difference quotient of 2nd order in y-dim at [it]
  real_t Grid::dyy(const Iterator &it) const{
      real_t numerator = _data[it.Top()] - 2.0 * _data[it] + _data[it.Down()];
      real_t dyy = 1.0/(_geom->Mesh()[1]*_geom->Mesh()[1])*numerator;
      return dyy;
  }

  /// Computes u*du/dx with the donor cell method
  real_t Grid::DC_udu_x(const Iterator &it, const real_t &alpha) const{
      real_t u_ij = _data[it];
      real_t u_left = _data[it.Left()];
      real_t u_right = _data[it.Right()];
      real_t dx = _geom->Mesh()[0];
      real_t DC_udu_x = 1.0/dx * ((u_ij+u_right)/2.0*(u_ij+u_right)/2.0 - (u_left+u_ij)/2.0*(u_left+u_ij)/2.0) \
                    + alpha/dx * ((abs(u_ij+u_right)/2.0 * abs(u_ij-u_right)/2.0) + (abs(u_ij+u_left)/2.0 * abs(u_left-u_ij)/2.0));
      return DC_udu_x;
  }
  /// Computes v*du/dy with the donor cell method
  real_t Grid::DC_vdu_y(const Iterator &it, const real_t &alpha, const Grid *v) const{
      real_t dy = _geom->Mesh()[1];
      real_t u_ij = _data[it];
      real_t u_top = _data[it.Top()];
      real_t u_down = _data[it.Down()];
      real_t v_ij = v->Cell(it);
      real_t v_down = v->Cell(it.Down());
      real_t v_rd = v->Cell((it.Right()).Down());
      real_t v_right = v->Cell(it.Right());

      real_t DC_vdu_y = 1.0/dy * ((v_ij+v_right)/2.0 * (u_ij+u_top)/2.0 - (v_down+v_rd)/2.0 * (u_down+u_ij)/2.0) \
                    + alpha/dy * (abs(v_ij+v_right)/2.0 * (u_ij-u_top)/2.0 - abs(v_down+v_rd)/2.0 * (u_down-u_ij)/2.0);
      return DC_vdu_y;
  }
  /// Computes u*dv/dx with the donor cell method
  real_t Grid::DC_udv_x(const Iterator &it, const real_t &alpha, const Grid *u) const{
      real_t dx = _geom->Mesh()[0];
      real_t v_ij = _data[it];
      real_t v_right = _data[it.Right()];
      real_t v_left = _data[it.Left()];
      real_t u_ij = u->Cell(it);
      real_t u_top = u->Cell(it.Top());
      real_t u_lt = u->Cell((it.Left()).Top());
      real_t u_left = u->Cell(it.Left());

      real_t DC_udv_x = 1.0/dx * ((u_ij+u_top)/2.0 * (v_ij+v_right)/2.0 - (u_left+u_lt)/2.0 * (v_left+v_ij)/2.0) \
                    + alpha/dx * (abs(u_ij+u_top)/2.0 * (v_ij-v_right)/2.0 - abs(u_left+u_lt)/2.0 * (v_left-v_ij)/2.0);
      return DC_udv_x;


  }
  /// Computes v*dv/dy with the donor cell method
    real_t Grid::DC_vdv_y(const Iterator &it, const real_t &alpha) const{
      real_t v_ij = _data[it];
      real_t v_down = _data[it.Down()];
      real_t v_top = _data[it.Top()];
      real_t dy = _geom->Mesh()[1];

      real_t DC_vdv_y = 1.0/dy * ((v_ij+v_top)/2.0*(v_ij+v_top)/2.0 - (v_down+v_ij)/2.0 * (v_down+v_ij)/2.0) \
                    + alpha/dy * ((abs(v_ij+v_top)/2.0 * abs(v_ij-v_top)/2.0) + (abs(v_ij+v_down)/2.0 * abs(v_down-v_ij)/2.0));
      return DC_vdv_y;
  }

  /// Returns the maximal value of the grid
  real_t Grid::Max() const{
      index_t dim = _geom->Size()[1]* _geom->Size()[0];
      real_t max_value = _data[0];
      for (index_t i=1; i<dim; ++i){
          if (max_value < _data[i]){
              max_value = _data[i];
          }
      }
      return max_value;
  }
  /// Returns the minimal value of the grid
  real_t Grid::Min() const{
      index_t dim = _geom->Size()[1]* _geom->Size()[0];
      real_t min_value = _data[0];
      for (index_t i=1; i<dim; ++i){
          if (min_value > _data[i]){
              min_value = _data[i];
          }
      }
      return min_value;
  }
  /// Returns the absolute maximal value
  real_t Grid::AbsMax() const{
      index_t dim = _geom->Size()[1]* _geom->Size()[0];
      real_t absmax_value = abs(_data[0]);
      for (index_t i=1; i<dim; ++i){
          if (absmax_value < abs(_data[i])){
              absmax_value = abs(_data[i]);
          }
      }
      return absmax_value;
  }

  /// Returns a pointer to the raw data
  real_t *Grid::Data(){
      return _data;
  }