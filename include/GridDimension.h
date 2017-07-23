#pragma once

#include "Vec.h"
#include <initializer_list>
#include <cstddef>

namespace hct
{
	template<unsigned int _D> struct GridDimension;

	template<>
	struct GridDimension<1> : public Vec<unsigned int, 1>
	{
		static constexpr unsigned int D = 1;
		inline GridDimension() : Vec<unsigned int,1>(1) {}
		inline GridDimension(Vec<unsigned int,1> v) : Vec<unsigned int,1>(v) {}
		inline GridDimension(unsigned int v) : Vec<unsigned int,1>(v) {}
		inline GridDimension(const unsigned int* l) : Vec<unsigned int, 1>(l) {}
		inline GridDimension(std::initializer_list<unsigned int> l) : Vec<unsigned int,1>(l) {}

		inline size_t branch(Vec<unsigned int,1> pos) const
		{
			return pos.val;
		}

		inline unsigned int gridSize() const
		{ 
			return this->reduce_mul();
		}

		inline GridDimension& operator = (Vec<unsigned int,1> v)
		{
			this->Vec<unsigned int,1>::operator = (v); return *this; 
		}
	};


  template<unsigned int _D>
    struct GridDimension : public Vec<unsigned int,_D>
    {
		static constexpr unsigned int D = _D;
      inline GridDimension() : Vec<unsigned int,D>(1) {}
      inline GridDimension(Vec<unsigned int,D> v) : Vec<unsigned int,D>(v) {}
	  inline GridDimension(const unsigned int* l) : Vec<unsigned int, D>(l) {}
	  inline GridDimension(unsigned int head, Vec<unsigned int,D-1> tail ) : Vec<unsigned int,D>(head,tail) {}
	  inline size_t branch(Vec<unsigned int, D> pos) const
	  {
		  GridDimension<D - 1> sgrid(*this);
		  return pos.val * sgrid.gridSize() + sgrid.branch(pos);
	  }
      inline unsigned int gridSize() const { return this->reduce_mul(); }
      inline GridDimension& operator = (Vec<unsigned int,D> v) { this->Vec<unsigned int,D>::operator = (v); return *this; }
    };

}
