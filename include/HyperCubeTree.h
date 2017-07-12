#pragma once

#include "HyperCubeTreeCell.h"
#include "TreeLevelStorage.h"
#include "GridDimension.h"

#include <cstddef>
#include <cstdint>

namespace hct
{


	/*
		Represent a tree traversal 'iterator'. basically stores the stack of parents up to the root.
		Note : together with a tree, this cursor enables to recreate local neighborhood if needed.
	*/
	template<unsigned int _D, std::size_t _MaxDepth=16>
	class HyperCubeTreeCursor
	{
		static constexpr unsigned int D = _D;
		static constexpr std::size_t MaxDepth = _MaxDepth;
		struct StackElement
		{
			Vec<std::size_t, D> branch; // position among siblings (relative to parent)
			std::size_t level;
			std::size_t index;
		};
		StackElement m_stack[MaxDepth];
		std::size_t m_depth;
	};

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

		static inline constexpr HyperCubeTreeCell rootCell() { return HyperCubeTreeCell(); }

	private:

		SubdivisionSchemeT m_subdivision_scheme;
		TreeLevelStorage m_storage;
		TreeLevelArray<int64_t> m_cell_child_index;
	};

}
