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
		inline GridDimension() : Vec<unsigned int, D>(1) {}
		inline GridDimension(const Vec<unsigned int, D>& v) : Vec<unsigned int, D>(v) {}
		inline GridDimension(std::initializer_list<unsigned int> l) : Vec<unsigned int, D>(l) {}

		inline size_t branch(const Vec<unsigned int, D>& pos) const
		{
			return pos.val;
		}
		inline unsigned int gridSize() const { return this->reduce_mul(); }
		inline GridDimension& operator = (const Vec<unsigned int, D>& v) { this->Vec<unsigned int, D>::operator = (v); return *this; }
	};


  template<unsigned int _D>
    struct GridDimension : public Vec<unsigned int,_D>
    {
		static constexpr unsigned int D = _D;
      inline GridDimension() : Vec<unsigned int,D>(1) {}
      inline GridDimension(const Vec<unsigned int,D>& v) : Vec<unsigned int,D>(v) {}
	  inline size_t branch(const Vec<unsigned int, D>& pos) const
	  {
		  GridDimension<D - 1> sgrid(*this);
		  return pos.val * sgrid.gridSize() + sgrid.branch(pos);
	  }
      inline unsigned int gridSize() const { return this->reduce_mul(); }
      inline GridDimension& operator = (const Vec<unsigned int,D>& v) { this->Vec<unsigned int,D>::operator = (v); return *this; }
    };

}
