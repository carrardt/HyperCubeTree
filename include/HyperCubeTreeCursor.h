#pragma once

#include "HyperCubeTreeCell.h"

namespace hct
{
	/*!
	All traversal cursor types must derive from this one
	or derive from HyperCubeTreeCell and have the same constructors as this one
	*/
	template<typename _Tree>
	struct HyperCubeTreeCursor : public HyperCubeTreeCell
	{
		using Tree = _Tree;
		static constexpr unsigned int D = Tree::D;
		using SubdivisionGrid = typename Tree::SubdivisionGrid;
		using GridLocation = typename Tree::GridLocation;

		inline HyperCubeTreeCursor() {}

		inline HyperCubeTreeCursor(HyperCubeTreeCell cell)
			: HyperCubeTreeCell(cell) {}

		inline HyperCubeTreeCursor(Tree& tree, HyperCubeTreeCell parent, SubdivisionGrid grid, GridLocation childLocation)
			: HyperCubeTreeCell(tree.child(parent, grid.branch(childLocation))) {}
	};

}
