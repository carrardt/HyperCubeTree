#pragma once

#include "HyperCubeTreeCell.h"
#include "TreeLevelStorage.h"
#include "GridDimension.h"
#include "Vec.h"
#include "GridEnum.h"

#include <cstddef>
#include <cstdint>

namespace hct
{
	/*!
	 level 0 corresponds to the single root cell
	 at each level, m_cell_child_index holds the first index where to find child locations in the next level.

	 exemples :
		a tree with only one root cell      ()
			Level0 [ -1 ]

		a tree with one root cell with 4 children 
			Level0 [ 0 ] 
				     |
			Level1 [ -1 -1 -1 -1 ] 

		a tree with root cell having 4 children, the second child has 2 child, so has the 4th.
		Level0 [  0 ]
		Level1 [ -1 0 -1 2 ]
		           /     /
		          |     |
		Level2 [ -1 -1 -1 -1]
	 */

	template<unsigned int _D, typename _SubdivisionSchemeT>
	class HyperCubeTree
	{
	public:
		using SubdivisionSchemeT = _SubdivisionSchemeT;
		static constexpr unsigned int D = _D;
		using GridLocation = Vec<unsigned int, D>;

		inline HyperCubeTree( SubdivisionSchemeT subdiv )
			: m_subdivision_scheme(subdiv)
		{
			m_storage.setNumberOfLevels( m_subdivision_scheme.getNumberOfLevelSubdivisions() + 1 );
			m_storage.resize(0, 1);
			m_storage.addArray(&m_cell_child_index);
			m_cell_child_index[rootCell()] = -1;
		}

		inline void addArray(ITreeLevelArray* a)
		{
			m_storage.addArray(a);
		}

		inline bool isLeaf(HyperCubeTreeCell cell) const
		{
			return m_cell_child_index[cell] < 0;
		}

		inline HyperCubeTreeCell child(HyperCubeTreeCell cell, size_t childIndex)
		{
			assert(!isLeaf(cell));
			assert((cell.m_level + 1) < m_storage.getNumberOfLevels());
			assert((m_cell_child_index[cell] + childIndex) < m_storage.getLevelSize(cell.m_level + 1));
			return HyperCubeTreeCell(cell.m_level + 1, m_cell_child_index[cell] + childIndex);
		}

		inline HyperCubeTreeCell child(HyperCubeTreeCell cell, GridLocation childLocation )
		{
			assert( cell.m_level < m_subdivision_scheme.getNumberOfLevelSubdivisions() );
			GridDimension<D> grid = m_subdivision_scheme.getLevelSubdivision(cell.m_level);
			assert((childLocation < grid).reduce_and());
			return child(cell, grid.branch(childLocation));
		}

		inline void refine(HyperCubeTreeCell cell)
		{
			assert(isLeaf(cell));
			assert(cell.m_level < m_subdivision_scheme.getNumberOfLevelSubdivisions());
			GridDimension<D> grid = m_subdivision_scheme.getLevelSubdivision(cell.m_level);
			size_t childStartIndex = m_storage.getLevelSize(cell.m_level + 1);
			m_storage.resize(cell.m_level + 1, childStartIndex + grid.gridSize());
			... remplir avec des -1
			m_cell_child_index[cell] = childStartIndex;
		}

		struct ChildFetchOperator
		{
			inline HyperCubeTreeCell operator () (HyperCubeTree& tree, HyperCubeTreeCell cell, GridLocation childLocation)
			{
				return tree.child(cell, childLocation);
			}
		};

		template<typename CellFuncT, typename CellCursorT=HyperCubeTreeCell, typename DigOperatorT=ChildFetchOperator>
		inline void preorderParseCells(CellFuncT f, DigOperatorT dig = DigOperatorT(), CellCursorT cursor = CellCursorT() )
		{
			f(cursor);
			if (!isLeaf(cursor))
			{
				GridDimension<D> grid = m_subdivision_scheme.getLevelSubdivision(cursor.m_level);
				ForEachGridLocation(grid, [&](GridLocation loc)
					{
						preorderParseCells( f, dig , dig(*this, cursor, loc) );
					}
				);
			}
		}

		template<typename StreamT>
		inline StreamT& toStream(StreamT & out)
		{
			m_subdivision_scheme.toStream(out);
			out << "root's child index : " << m_cell_child_index[rootCell()] << '\n';
			return m_storage.toStream(out);
		}

		static inline constexpr HyperCubeTreeCell rootCell() { return HyperCubeTreeCell(0,0); }

	private:

		SubdivisionSchemeT m_subdivision_scheme;
		TreeLevelStorage m_storage;
		TreeLevelArray<int64_t> m_cell_child_index;
	};

}
