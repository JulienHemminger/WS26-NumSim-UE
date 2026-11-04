#include "solver.hpp"
#include "grid.hpp"
#include "geometry.hpp"
#include "iterator.hpp"

//------------------------------------------------------------------------------
/** abstract base class for an iterative solver
*/
/// Constructor of the abstract Solver class
  Solver::Solver(const Geometry *geom){
    _geom = geom;
  }
  /// Destructor of the Solver Class
  Solver::~Solver(){
    delete _geom;
  }
  /// This function must be implemented in a child class
  // @param [in][out] grid current values
  // @param [in]      rhs  right hand side values
  // @returns accumulated residual
  real_t Solver::Cycle(Grid *grid, const Grid *rhs) const{
    std::cerr << "The abstract Cycle() function does not operate. One must invoke and use a concrete solver." << '\n';
  }
  /// Returns the residual at [it] for the pressure-Poisson equation
  // Output in a nutshell: residue_at_it = rhs[it] - laplace(pressure[it])
  real_t Solver::localRes(const Iterator &it, const Grid *grid, const Grid *rhs) const{
    return grid->dxx(it) + grid->dyy(it) - rhs->Cell(it);
  }

//------------------------------------------------------------------------------
  /** concrete SOR solver
*/
/// Constructs an actual SOR solver
  SOR::SOR(const Geometry *geom, const real_t &omega):Solver(geom){
    _geom = geom;
    _omega = omega;
  }
  /// Destructor
  SOR::~SOR(){
    delete _geom;
  }

  /// Returns the total residual (using 2-norm) and executes a solver cycle
  // @param grid current pressure values
  // @param rhs right hand side
  real_t SOR::Cycle(Grid *grid, const Grid *rhs) const{
    real_t dx = _geom->Mesh()[0];
    real_t dy = _geom->Mesh()[1];
    InteriorIterator it(_geom); // invokes interior iterator
    it.First();
    real_t local_res;
    real_t residue = 0.0;
    real_t temp = dx*dx*dy*dy / (2.0*(dx*dx+dy*dy));
    while(it.Valid()){
      local_res = this->localRes(it,grid,rhs);
      residue = residue + local_res*local_res;
      grid->Cell(it) = grid->Cell(it) + (_omega * temp * local_res); // solve pressure equation
      it.Next();
    }
    return sqrt(residue*dx*dy); // Euclidean norm of the residue
  }
