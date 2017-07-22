#pragma once

#include "HyperCubeTreeCursor.h"
#include "HyperCubeTreeCell.h"
#include "Vec.h"

namespace hct
{

/*
A tree traversal cursor adding information about position and size of the traversed cell.
*/
template<typename _Tree>
struct HyperCubeTreeLocatedCursor : public hct::HyperCubeTreeCursor<_Tree>
{
	using SuperClass = hct::HyperCubeTreeCursor<_Tree>;
	using Tree = _Tree;
	static constexpr unsigned int D = Tree::D;
	using VecT = hct::Vec<double, D>;
	using SubdivisionGrid = typename Tree::SubdivisionGrid;
	using GridLocation = typename Tree::GridLocation;

	// initialization constructors
	inline HyperCubeTreeLocatedCursor(hct::Vec<double, D> domainSize, hct::HyperCubeTreeCell cell = hct::HyperCubeTreeCell())
		: SuperClass(cell)
		, m_origin(0.0)
		, m_size(domainSize) {}

	// recursion constructor
	inline HyperCubeTreeLocatedCursor(const Tree& tree, HyperCubeTreeLocatedCursor parent, SubdivisionGrid grid, GridLocation childLocation)
		: SuperClass(tree, parent, grid, childLocation)
	{
		m_size = parent.m_size / grid;
		m_origin = parent.m_origin + childLocation * m_size;
	}

	VecT m_origin;
	VecT m_size;
};

}