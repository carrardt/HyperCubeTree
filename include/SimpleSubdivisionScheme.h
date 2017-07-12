#pragma once

#include "GridDimension.h"
#include <vector>

namespace hct
{
	template<unsigned int _D>
	class SimpleSubdivisionScheme
	{
		static constexpr unsigned int D = _D;
	public:
		inline void addLevelSubdivision(GridDimension<D> grid)
		{
			m_level_subdivisions.push_back(grid);
		}

		inline size_t getNumberOfLevelSubdivisions() const
		{
			return m_level_subdivisions.size();
		}

		inline GridDimension<D> getLevelSubdivision(size_t level) const
		{
			assert(level < m_level_subdivisions.size());
			return m_level_subdivisions[level];
		}
	private:
		std::vector< GridDimension<D> > m_level_subdivisions;
	};

}