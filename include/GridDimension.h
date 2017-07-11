#pragma once

#include "Vec.h"

namespace hct
{

  template<unsigned int D>
    struct GridDimension : public Vec<unsigned int,D>
    {
      inline GridDimension() : Vec<unsigned int,D>(1) {}
      inline GridDimension(const Vec<unsigned int,D>& v) : Vec<unsigned int,D>(v) {}
      inline unsigned int gridSize() const { return this->reduce_mul(); }
      inline unsigned int branch( const Vec<unsigned int,D>& pos ) const { return this->gridIndex(pos); }
      inline GridDimension& operator = (const Vec<unsigned int,D>& v) { this->Vec<unsigned int,D>::operator = (v); return *this; }
    };
    
}
