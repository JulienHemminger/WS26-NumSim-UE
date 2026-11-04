#include "iterator.hpp"
#include "geometry.hpp"

//------------------------------------------------------------------------------
/** Iterator base class
*/
/// Constructs a new Iterator depending on a geometry
  Iterator::Iterator(const Geometry *geom):Iterator(geom,0){

  }
  /// Constructs a new Iterator on a geometry with a defined starting value
  Iterator::Iterator(const Geometry *geom, const index_t &value){
      _geom = geom;
      _value = value;
      _valid = true;
  }

  ///     Returns the current position value
  const index_t &Iterator::Value() const{
      return _value;
  }
  /// Cast operator to convert Iterators to integers
  Iterator::operator const index_t &() const{
      return _value;
  }
  /// Returns the position coordinates
  multi_index_t Iterator::Pos() const{
      multi_index_t position;
      position[0] = _value % _geom->Size()[0];
      position[1] = _value / _geom->Size()[0];
      return position;
  }

  /// Sets the iterator to the first element
  void Iterator::First(){
      _value = 0;
  }
  /// Goes to the next element of the iterator, disables it if position is end
  void Iterator::Next(){
      multi_index_t geosize = _geom->Size();
      index_t ende = geosize[0]*geosize[1]-1;
      _value = _value + 1;
      if (_value>ende){
          _valid = false;
      }
  }

  /// Checks if the iterator still has a valid value
  bool Iterator::Valid() const{
      return _valid;
  }

  /// Returns an Iterator that is located left from this one.
  // if we are at the left boundary, the cell sees itself
  Iterator Iterator::Left() const{
      multi_index_t position = Pos();
      if (position[0]==0){
          return Iterator(_geom,_value);
      }else{
          return Iterator(_geom,_value-1);
      }
  }

  /// Returns an Iterator that is located right from this one
  // If we are at the right boundary, the cell sees itself
  Iterator Iterator::Right() const{
      multi_index_t position = Pos();
      multi_index_t geosize = _geom->Size();
      if (position[0]==geosize[0]-1){
          return Iterator(_geom,_value);
      }else{
          return Iterator(_geom,_value+1);
      }
  }

  /// Returns an Iterator that is located above this one
  // If we are at the upper domain boundary, the cell sees itself
  Iterator Iterator::Top() const{
      multi_index_t position = Pos();
      multi_index_t geosize = _geom->Size();
      if (position[1]==geosize[1]-1){
          return Iterator(_geom,_value);
      }else{
          return Iterator(_geom,_value+geosize[0]);
      }
  }

  /// Returns an Iterator that is located below this one
  // If we are at the lower domain boundary, the cell sees itself
  Iterator Iterator::Down() const{
      multi_index_t position = Pos();
      multi_index_t geosize = _geom->Size();
      if (position[1]==0){
          return Iterator(_geom,_value);
      }else{
          return Iterator(_geom,_value-geosize[0]);
      }
  }

  //------------------------------------------------------------------------------
/** Iterator for interior cells
*/
/// Construct a new InteriorIterator
  InteriorIterator::InteriorIterator(const Geometry *geom):Iterator(geom){ // using initializer list as Grid has no default constructor
  }


  /// Sets the iterator to the first element
  void InteriorIterator::First(){
      multi_index_t geosize = _geom->Size();
      _value = geosize[0] + 1;
  }
  /// Goes to the next element of the iterator, disables it if position is end
  void InteriorIterator::Next(){
    _value = _value + 1;
    if (_value > (_geom->Size()[0] * _geom->Size()[1]) - 1){
       _valid = false;
    }// Right Boundary
    if (Pos()[0] == _geom->Size()[0] - 1 && _valid == true){
        Next();
    }// Left Boundary
    else if (Pos()[0] == 0 && _valid == true){
        Next();
    }// Bottom Boundary
    else if (Pos()[1] == 0 && _valid == true){
        Next();
    }// Top Boundary
    else if (Pos()[1] == _geom->Size()[1] - 1 && _valid == true){
        Next();
    }
  }


//------------------------------------------------------------------------------
/** Iterator for domain boundary cells.
*/
/// Constructs a new BoundaryIterator
  BoundaryIterator::BoundaryIterator(const Geometry *geom):Iterator(geom){ // using initializer list as Grid has no default constructor
      _geom = geom;
      _boundary = 0;
  }

  /// Sets the boundary to iterate (The corners are excluded)
  //          2
  //    -------------
  //    |           |
  //    |           |
  //  3 |           | 1
  //    |           |
  //    |           |
  //    -------------
  //          0
  void BoundaryIterator::SetBoundary(const index_t &boundary){
      _boundary = boundary;
  }

  /// Sets the iterator to the first element
  void BoundaryIterator::First(){
      if ( _boundary==0){
          _value = 0;
      }else if (_boundary==1){
              _value = _geom->Size()[0]-1;
        }else if (_boundary==2){
              _value = _geom->Size()[0]*(_geom->Size()[1]-1);
          }else if (_boundary==3){
              _value = 0;
          }
  }
  /// Goes to the next element of the iterator, disables it if position is end
  void BoundaryIterator::Next(){
      index_t tmp = 0;

      if (_boundary == 0){
          //lower boundary
         tmp = Right().Value(); 
      }
      else if (_boundary == 1){
          //right boundary
         tmp = Top().Value(); 
      }
      else if (_boundary == 2){
          //upper boundary
         tmp = Right().Value(); 
      }
      else if (_boundary == 3){
          //left boundary
         tmp = Top().Value(); 
      }

      if (tmp == _value){
          _valid = false;
      }
      else{
          _value = tmp;
      }
  }