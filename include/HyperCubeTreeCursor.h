#pragma once

#include "HyperCubeTreeCell.h"

namespace hct
{
	/*!
	All traversal cursor types must derive from this one
	or derive from HyperCubeTreeCell and have the same constructors as this one
	*/
	template<typename _Tree>
	class HyperCubeTreeCursor
	{
		public:
		
		using Tree = _Tree;
		static constexpr unsigned int D = Tree::D;
		using SubdivisionGrid = typename Tree::SubdivisionGrid;
		using GridLocation = typename Tree::GridLocation;

		inline HyperCubeTreeCursor(HyperCubeTreeCell cell = HyperCubeTreeCell() )
			: m_cell(cell) {}

		inline HyperCubeTreeCursor(Tree& tree, HyperCubeTreeCursor parent, SubdivisionGrid grid, GridLocation childLocation)
			: m_cell(tree.child(parent.cell(), grid.branch(childLocation))) {}
		
		inline HyperCubeTreeCell cell() const { return m_cell; }
		
		private:
		HyperCubeTreeCell m_cell;
	};

}
