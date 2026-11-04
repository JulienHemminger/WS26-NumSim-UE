#include "typedef.hpp"
#include "parameter.hpp"
#include "geometry.hpp"
#include "grid.hpp"
#include "iterator.hpp"

/// Geometry constructor
Geometry::Geometry(){
    Load("default.geom");
    _size[0] = _size[0] + 2; // for ghostpoints at boundary
    _size[1] = _size[1] + 2; // for ghostpoints
    _h[0] = _length[0]/(_size[0]-2);
    _h[1] = _length[1]/(_size[0]-2);
}

/// Loads a geometry from a file
void Geometry::Load(const char *file){
    std::ifstream cFile;
    cFile.open(file);
    if(cFile.is_open()){
        std::string line;
        while(std::getline(cFile,line)){
            auto delimiterPos1 = line.find("=");
            std::string name = line.substr(0, delimiterPos1);
            std::string value = line.substr(delimiterPos1 +1);
        
            if (name == "pressure"){
                std::string value1 = value;
                std::string value2 = "0.00000"; // pressure has no second value to be considered
                setGeoVectors(name,value1, value2);
                std::cout << name << " " << value1 << " end \n";
            }
            else {
                auto delimiterPos2 = value.find("_");
                std::string value1 = value.substr(0,delimiterPos2);
                std::string value2 = value.substr(delimiterPos2 + 1);
                // set input strings to doubles in workspace
                setGeoVectors(name,value1, value2);
                std::cout << name << " " << value1 << " " << value2 << " end \n";
            }
            if(cFile.eof()) break;
        }
    }
    else {
        std::cerr << "Couldn't open default geometry file for reading.\n";
    }
    cFile.close();
}

/// turn input strings to doubles
void Geometry::setGeoVectors(std::string name, std::string value1, std::string value2){
    if (name == "size"){
        _size[0] = ::atof(value1.c_str());
        _size[1] = ::atof(value2.c_str());
    }
    if (name == "length"){
        _length[0] = ::atof(value1.c_str());
        _length[1] = ::atof(value2.c_str());
    }
    if (name == "velocity"){
        _velocity[0] = ::atof(value1.c_str());
        _velocity[1] = ::atof(value2.c_str());
    }
    if (name == "pressure"){
        _pressure = ::atof(value1.c_str());
    }
}

/// Returns the number of cells in each dimension
const multi_index_t &Geometry::Size() const{
    return _size;
}
/// Returns the length of the domain
const multi_real_t &Geometry::Length() const{
    return _length;
}
/// Returns the meshwidth
const multi_real_t &Geometry::Mesh() const{
    return _h;
}

/// Updates the velocity field u
void Geometry::Update_U(Grid *u) const{

    // Left boundary
    BoundaryIterator b_it_l(this);
    b_it_l.SetBoundary(3);
    b_it_l.First();
        while(b_it_l.Valid()){
            u->Cell(b_it_l) = 0.0;
            b_it_l.Next();
        } 
    
    //VisualizeGrid(u);

    // Right boundary
    BoundaryIterator b_it_r(this);
    b_it_r.SetBoundary(1);
    b_it_r.First();
        while(b_it_r.Valid()){
            u->Cell(b_it_r) = 0.0;
            u->Cell(b_it_r.Left()) = 0.0;
            b_it_r.Next();
        }
    
    //VisualizeGrid(u);

    // Bottom boundary
    BoundaryIterator b_it_d(this);
    b_it_d.SetBoundary(0);
    b_it_d.First();
        while(b_it_d.Valid()){
            u->Cell(b_it_d) = - u->Cell(b_it_d.Top());
            b_it_d.Next();
        }
    
    //VisualizeGrid(u);

    // Top boundary
    BoundaryIterator b_it_t(this);
    b_it_t.SetBoundary(2);
    b_it_t.First();
        while(b_it_t.Valid()){
            real_t u_b = 1.0; // u velocity on boundary line
            u->Cell(b_it_t) = 2.0*u_b - u->Cell(b_it_t.Down());
            b_it_t.Next();
        }
    
    //VisualizeGrid(u);

}
/// Updates the velocity field v
void Geometry::Update_V(Grid *v) const{
    
    // Left boundary
    BoundaryIterator b_it_l(this);
    b_it_l.SetBoundary(3);
    b_it_l.First();
        while(b_it_l.Valid()){
            v->Cell(b_it_l) = - v->Cell(b_it_l.Right());
            b_it_l.Next();
        } 
    

    // Right boundary
    BoundaryIterator b_it_r(this);
    b_it_r.SetBoundary(1);
    b_it_r.First();
        while(b_it_r.Valid()){
            v->Cell(b_it_r) = - v->Cell(b_it_r.Left());
            b_it_r.Next();
        }
    

    // Bottom boundary
    BoundaryIterator b_it_d(this);
    b_it_d.SetBoundary(0);
    b_it_d.First();
        while(b_it_d.Valid()){
            v->Cell(b_it_d) = 0.0;
            b_it_d.Next();
        }
    

    // Top boundary
    BoundaryIterator b_it_t(this);
    b_it_t.SetBoundary(2);
    b_it_t.First();
        while(b_it_t.Valid()){
            v->Cell(b_it_t) = 0.0;
            v->Cell(b_it_t.Down()) = 0.0;
            b_it_t.Next();
        }
    

}
/// Updates the pressure field p
void Geometry::Update_P(Grid *p) const{
    
    // Left boundary
    BoundaryIterator b_it_l(this);
    b_it_l.SetBoundary(3);
        b_it_l.First();
        while(b_it_l.Valid()){
            p->Cell(b_it_l) = p->Cell(b_it_l.Right());
            b_it_l.Next();
        } 
    

    // Right boundary
    BoundaryIterator b_it_r(this);
    b_it_r.SetBoundary(1);
    b_it_r.First();
        while(b_it_r.Valid()){
            p->Cell(b_it_r) = p->Cell(b_it_r.Left());
            b_it_r.Next();
        }
    

    // Bottom boundary
    BoundaryIterator b_it_d(this);
    b_it_d.SetBoundary(0);
    b_it_d.First();
        while(b_it_d.Valid()){
            p->Cell(b_it_d) = p->Cell(b_it_d.Top());
            b_it_d.Next();
        }
    

    // Top boundary
    BoundaryIterator b_it_t(this);
    b_it_t.SetBoundary(2);
    b_it_t.First();
        while(b_it_t.Valid()){
            p->Cell(b_it_t) = p->Cell(b_it_t.Down());
            b_it_t.Next();
        }
    
}

/// for debug purposes
  void Geometry::VisualizeGrid(const Grid *grid) const{
    Iterator it(this);
    it.First();
    while(it.Valid()){
      std::cout << std::setw (2) << grid->Cell(it) << "  ";
      it.Next();
      if(it%this->Size()[0] == 0){
        std::cout << std::endl;
      }
    }
    std::cout << std::endl;
  }