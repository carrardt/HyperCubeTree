#pragma once

#include "Vec.h"
#include <cstddef>

namespace hct
{
	template <unsigned int D>
	struct HyperCubeTreeCellPosition
	{
		using VecI = Vec<size_t, D>;
		using VecF = Vec<double, D>;

		VecI m_position; // position
		VecI m_resolution; // resolution at wich position is expressed

		inline bool operator == (const HyperCubeTreeCellPosition& v) const
		{
			VecI p1 = m_position * v.m_resolution;
			VecI p2 = v.m_position * m_resolution;
			return (p1 == p2).reduce_and();
		}

		// ordering operator
		inline bool operator < (const HyperCubeTreeCellPosition& v) const
		{
			VecI p1 = m_position * v.m_resolution;
			VecI p2 = v.m_position * m_resolution;
			return p1.less(p2);
		}

		inline HyperCubeTreeCellPosition refine(const Vec<size_t, D>& grid) const
		{
			return HyperCubeTreeCellPosition{ m_position*grid , m_resolution*grid };
		}

		inline HyperCubeTreeCellPosition operator + (const Vec<size_t, D>& offset) const
		{
			return HyperCubeTreeCellPosition{ m_position+offset , m_resolution };
		}

		inline VecF normalize() const
		{
			return VecF(m_position) / m_resolution;
		}

		// increment position by half a unit step in all directions
		inline HyperCubeTreeCellPosition addHalfUnit() const
		{
			return HyperCubeTreeCellPosition{ m_position*2 + 1 , m_resolution*2 };
		}

		template<typename StreamT>
		inline StreamT& toStream(StreamT& out)
		{
			return normalize().toStream(out);
		}

	};


}