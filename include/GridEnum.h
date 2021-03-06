#pragma once

#include "Vec.h"
#include "GridDimension.h"

namespace hct
{
	template <unsigned int D, unsigned int D2, typename ProcObj> struct GridEnum;
	template <unsigned int D, unsigned int D2, typename ProcObj> struct GridEnumNoRef;

	template <unsigned int D2, typename ProcObj>
	struct GridEnum<0, D2, ProcObj>
	{
		static inline void enumerate(GridDimension<0>, const Vec<unsigned int, D2> coord, ProcObj& proc)
		{
			proc(coord.reverse());
		}
	};

	template <unsigned int D2, typename ProcObj>
	struct GridEnumNoRef<0, D2, ProcObj>
	{
		static inline void enumerateNoRef(GridDimension<0>, const Vec<unsigned int, D2> coord, ProcObj proc)
		{
			proc(coord.reverse());
		}
	};

	template <unsigned int D, unsigned int D2, typename ProcObj>
	struct GridEnum
	{
		static inline void enumerate(const GridDimension<D>& grid, const Vec<unsigned int, D2> coord, ProcObj& proc)
		{
			for (unsigned int i = 0; i < grid.val; i++)
			{
				GridEnum<D - 1, D2 + 1, ProcObj>::enumerate(grid, Vec<unsigned int, D2 + 1>(i, coord), proc);
			}
		}
	};

	template <unsigned int D, unsigned int D2, typename ProcObj>
	struct GridEnumNoRef
	{
		static inline void enumerateNoRef(GridDimension<D> grid, Vec<unsigned int, D2> coord, ProcObj proc)
		{
			for (unsigned int i = 0; i < grid.val; i++)
			{
				GridEnumNoRef<D - 1, D2 + 1, ProcObj>::enumerateNoRef(grid, Vec<unsigned int, D2 + 1>(i, coord), proc);
			}
		}
	};

	template <unsigned int D, typename ProcObj>
	static inline void gridEnum(const GridDimension<D>& grid, ProcObj& proc)
	{
		GridEnum<D, 0, ProcObj>::enumerate(grid, Vec<unsigned int, 0>(), proc);
	}

	template <unsigned int D, typename ProcObj>
	static inline void ForEachGridLocation(GridDimension<D> grid, ProcObj proc)
	{
		GridEnumNoRef<D, 0, ProcObj>::enumerateNoRef(grid, Vec<unsigned int, 0>(), proc);
	}

}; // hct
