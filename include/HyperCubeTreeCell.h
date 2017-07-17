#pragma once

#include <cstddef>
#include <limits>

namespace hct
{
	struct HyperCubeTreeCell
	{
		inline HyperCubeTreeCell() : m_level(0), m_index(0) {}
		inline HyperCubeTreeCell(size_t l, size_t i) : m_level(l), m_index(i) {}

		inline size_t level() const { return m_level; }
		inline size_t index() const { return m_index; }

		inline bool operator == (const HyperCubeTreeCell& cell) const
		{
			return m_level == cell.level() && m_index == cell.index();
		}

		// returns true if this cell belongs to a tree.
		inline bool isTreeCell() const
		{
			return (m_level>0) || (m_index==0);
		}

		inline bool isNil() const
		{
			return operator == (nil());
		}

		template<typename StreamT>
		inline StreamT& toStream(StreamT& out)
		{
			out << m_level << ':' << m_index;
			return out;
		}

		static HyperCubeTreeCell nil()
		{
			return HyperCubeTreeCell( 0, std::numeric_limits<size_t>::max() );
		}

	private:
		size_t m_level;
		size_t m_index;
	};

}
