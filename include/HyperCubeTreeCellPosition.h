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

		inline HyperCubeTreeCellPosition() : m_position(0), m_resolution(1) {}
		inline HyperCubeTreeCellPosition(VecI p, VecI r) : m_position(p), m_resolution(r) {}

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

		inline bool boundary() const
		{
			return (m_position == VecI(0)).reduce_or() || (m_position == m_resolution).reduce_or();
		}

		template<typename StreamT>
		inline StreamT& toStream(StreamT& out) const
		{
			m_position.toStream(out);
			out << "/";
			m_resolution.toStream(out);
			return out;
		}

		VecI m_position; // position
		VecI m_resolution; // resolution at wich position is expressed
	};


}