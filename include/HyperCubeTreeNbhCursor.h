#pragma once

#include "HyperCubeTreeCell.h"
#include "HyperCube.h"

namespace hct
{
	/*
	A tree traversal cursor adding information about position and size of the traversed cell.
	*/
	template<typename _Tree>
	struct HyperCubeTreeNbhCursor
	{
		using Tree = _Tree;
		static constexpr unsigned int D = Tree::D;
		using HCube = HyperCube< hct::HyperCubeTreeCell , D >;
		using SubdivisionGrid = typename Tree::SubdivisionGrid;
		using GridLocation = typename Tree::GridLocation;

		// initialization constructors
		inline HyperCubeTreeNbhCursor(hct::HyperCubeTreeCell cell = hct::HyperCubeTreeCell())
		{
			hct::HyperCubeTreeCell nullCell(0, 1); // detected "external" of any tree
			m_nbh.forEachValue([=nullCell](hct::HyperCubeTreeCell& cell) { cell = nullCell; })
			m_nbh.self() = cell;
		}

		// recursion constructor
		inline HyperCubeTreeNbhCursor(Tree& tree, HyperCubeTreeLocatedCursor parent, SubdivisionGrid grid, GridLocation childLocation)
		{
			
		}

		HCube m_nbh;
	};

}
