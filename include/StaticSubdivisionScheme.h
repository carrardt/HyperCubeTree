#pragma once

#include "Vec.h"

namespace hct
{

	// ==================== StaticGrid =======================
	/* A type holding a list of integers, representing the dimensions of a grid
	*/
	template<unsigned int... S>
	struct StaticGridDim
	{
		static constexpr unsigned int D = sizeof...(S);
		static inline Vec<unsigned int,D> value()
		{
			return Vec<unsigned int, D>({ S... });
		}
	};

	template<typename... GridTypes>
	struct StaticGridDimSet
	{
		static constexpr size_t grid_size = 0;
		static constexpr bool is_valid = false;
	};
	template<unsigned int... S>
	struct StaticGridDimSet< StaticGridDim<S...> >
	{
		static constexpr size_t grid_size = sizeof...(S);
		static constexpr bool is_valid = true;
	};

	template<unsigned int... S, typename... GridTypes>
	struct StaticGridDimSet< StaticGridDim<S...> , GridTypes... >
	{
		static constexpr size_t grid_size = sizeof...(S);
		static constexpr bool is_valid = StaticGridDimSet<GridTypes...>::is_valid && StaticGridDimSet<GridTypes...>::grid_size==grid_size;
	};

}
