  #include "compute.hpp"
  #include "geometry.hpp"
  #include "parameter.hpp"
  #include "grid.hpp"
  #include "iterator.hpp"
  #include "solver.hpp"
  
  /// Creates a compute instance with given geometry and parameter
  Compute::Compute(const Geometry *geom, const Parameter *param){
    _geom = geom;
    _param = param;
    _t = 0.0;
    multi_real_t u_offset = {_geom->Mesh()[0], 0.5*_geom->Mesh()[1]};
    multi_real_t v_offset = {0.5*_geom->Mesh()[0], _geom->Mesh()[1]};
    multi_real_t p_offset = {0.5*_geom->Mesh()[0], 0.5*_geom->Mesh()[1]};
    multi_real_t vort_offset = {_geom->Mesh()[0], _geom->Mesh()[1]}; // "normal" grid
    multi_real_t stream_offset = {_geom->Mesh()[0], _geom->Mesh()[1]};
    _u = new Grid(_geom, u_offset);
    _v = new Grid(_geom, v_offset);
    _p = new Grid(_geom, p_offset);
    _vort = new Grid(_geom, vort_offset);
    _stream = new Grid(_geom, stream_offset);
    _F = new Grid(_geom, u_offset);
    _G = new Grid(_geom, v_offset);
    _rhs = new Grid(_geom);
    _tmp = new Grid(_geom, vort_offset);
    
    //determine solver
    _solver = new SOR(_geom, _param->Omega());

    // initialize
    _u->Initialize(0.0);
    _v->Initialize(0.0);
    _p->Initialize(0.0);
    _vort->Initialize(0.0);
    _stream->Initialize(0.0);
    _F->Initialize(0.0);
    _G->Initialize(0.0);
    _rhs->Initialize(0.0);
    _tmp->Initialize(0.0);

    // boundary conditions
    // VisualizeGrid(_u);
    _geom->Update_U(_u);
    _geom->Update_V(_v);
    _geom->Update_U(_F);
    _geom->Update_V(_G);

  }
  /// Deletes all grids
  Compute::~Compute(){
    delete _u;
    delete _v;
    delete _p;
    delete _vort;
    delete _stream;
    delete _F;
    delete _G;
    delete _rhs;
    delete _tmp;

  }

  /// Execute one time step of the fluid simulation (with or without debug info)
  // @ param printInfo print information about current solver state (residual
  // etc.)
  void Compute::TimeStep(bool printInfo){
    double u_max = _u->AbsMax();
    double v_max = _v->AbsMax();
    real_t dx = _geom->Mesh()[0];
    real_t dy = _geom->Mesh()[1];
    // Calculate timestep
    real_t diff = _param->Re()/2 * dx *dx *dy*dy/(dx*dx+dy*dy);
    real_t conv = std::min(dx/u_max, dy/v_max);
    real_t _dtlimit = _param->Tau() * std::min(diff,conv);
    
    if(_param->Dt()>0){
      _dtlimit = _param->Dt();
    }

    // VisualizeGrid(_u);
    // F and G
    MomentumEqu(_dtlimit);

    // Update all grids and calculate rhs
    _geom->Update_U(_F);
    _geom->Update_V(_G);
    RHS(_dtlimit);

    // set residual and iteration limit
    real_t _epslimit = _param->Eps();
    index_t _itermax = _param->IterMax();
    real_t _epsc = _epslimit + 1;
    index_t _iter = 0;

    //Output infos
    if(printInfo){
      std::cout << "==============================================" << std::endl;
    	std::cout << "Time stepsize: " << _dtlimit << std::endl;
    	std::cout << "Current time step: " << _t << " of " << _param->Tend() << std::endl;
    }

    _geom->Update_P(_p);
    while (_iter < _itermax && _epsc > _epslimit){
      _epsc = _solver->Cycle(_p,_rhs);
      _geom->Update_P(_p);

      if(printInfo && _epsc <= _epslimit){
        std::cout << "Solver converged. Iterations: " << _iter+1 << std::endl;
      }
      _iter++;
    }

    NewVelocities(_dtlimit);
    _geom->Update_U(_u);
    _geom->Update_V(_v);

    _t = _t + _dtlimit;

          /// Printinfo
      if(printInfo){
    	  std::cout << "Current residual: " << _epsc << std::endl;
          std::cout << "==============================================" << "\n" << std::endl;
      }

  }

/// for debug purposes
  void Compute::VisualizeGrid(const Grid *grid){
    Iterator it(_geom);
    it.First();
    while(it.Valid()){
      std::cout << std::setw (2) << grid->Cell(it) << "  ";
      it.Next();
      if(it%_geom->Size()[0] == 0){
        std::cout << std::endl;
      }
    }
    std::cout << std::endl;
  }

  /// Returns the simulated time in total
  const real_t &Compute::GetTime() const{
    return _t;
  }

  /// Returns the pointer to U
  const Grid *Compute::GetU() const{
    return _u;
  }
  /// Returns the pointer to V
  const Grid *Compute::GetV() const{
    return _v;
  }
  /// Returns the pointer to P
  const Grid *Compute::GetP() const{
    return _p;
  }
  /// Returns the pointer to RHS
  const Grid *Compute::GetRHS() const{
    return _rhs;
  }

  /// Computes and returns the absolute velocity
  const Grid *Compute::GetVelocity(){
    Iterator it(_geom);
    it.First();
    while(it.Valid()){
      _tmp->Cell(it) = sqrt(_u->Cell(it)*_u->Cell(it) + _v->Cell(it)*_v->Cell(it));
      it.Next();
    }
    return _tmp;
  }
  /// Computes and returns the vorticity
  const Grid *Compute::GetVorticity(){
    Iterator it(_geom);
    it.First();
    while(it.Valid()){
      _vort->Cell(it) = _u->dy_r(it) - _v->dx_r(it);
      it.Next();
    }
    return _vort;
  }
  /// Computes and returns the stream line values
  const Grid *Compute::GetStream(){
    Iterator it(_geom);
    it.First();
    while(it.Valid()){
      if(it.Pos()[1] == 0){
        _stream->Cell(it) = _stream->Cell(it.Left()) - _v->Cell(it)*_geom->Mesh()[0];
      }
      else{
        _stream->Cell(it) = _stream->Cell(it.Down()) + _u->Cell(it)*_geom->Mesh()[1];
      }
      it.Next();
    }
    return _stream;
  }

  /// Compute the new velocites u,v
  void Compute::NewVelocities(const real_t &dt){
    // using F and G for calculating new velocities
    InteriorIterator it(_geom);
    while(it.Valid()){
      _u->Cell(it) = _F->Cell(it) - dt * _p->dx_r(it);
      _v->Cell(it) = _G->Cell(it) - dt * _p->dy_r(it);
      it.Next();
    }
  }
  /// Compute the temporary velocites F,G
  void Compute::MomentumEqu(const real_t &dt){
    InteriorIterator it(_geom);
    while(it.Valid()){
      _F->Cell(it) = _u->Cell(it) + dt * (1.0 / _param->Re() * (_u->dxx(it) + _u->dyy(it)) - _u->DC_udu_x(it, _param->Alpha()) \
       - _u->DC_vdu_y(it, _param->Alpha(), _v));
      _G->Cell(it) = _v->Cell(it) + dt * (1.0 / _param->Re() * (_v->dxx(it) + _v->dyy(it)) - _v->DC_vdv_y(it, _param->Alpha()) \
       - _v->DC_udv_x(it, _param->Alpha(), _u));
      it.Next();
    }
  }
  /// Compute the RHS of the poisson equation
  void Compute::RHS(const real_t &dt){
    InteriorIterator it(_geom);
    while(it.Valid()){
      _rhs->Cell(it) = 1.0 / dt * (_F->dx_l(it) + _G->dy_l(it));
      it.Next();
    }
  }

