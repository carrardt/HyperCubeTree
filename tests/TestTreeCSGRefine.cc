#include "HyperCubeTree.h"
#include "SimpleSubdivisionScheme.h"
#include "GridDimension.h"
#include "csg.h"

#include <iostream>
#include <algorithm>
#include <initializer_list>
#include <cmath>
#include <chrono>

/*
	A tree traversal cursor adding information about position and size of the traversed cell.
*/
template<typename _Tree>
struct TreeCursorWithPosition : public hct::HyperCubeTreeCursor<_Tree>
{
	using SuperClass = hct::HyperCubeTreeCursor<_Tree>;
	using Tree = _Tree;
	static constexpr unsigned int D = Tree::D;
	using VecT = hct::Vec<double,D>;
	using SubdivisionGrid = typename Tree::SubdivisionGrid;
	using GridLocation = typename Tree::GridLocation;

	// initialization constructors
	inline TreeCursorWithPosition(hct::Vec<double,D> domainSize , hct::HyperCubeTreeCell cell = hct::HyperCubeTreeCell() )
		: SuperClass(cell)
		, m_origin(0.0)
		, m_size(domainSize) {}

	// recursion constructor
	inline TreeCursorWithPosition(Tree& tree, TreeCursorWithPosition parent, SubdivisionGrid grid, GridLocation childLocation)
		: SuperClass(tree, parent, grid, childLocation)
	{
		m_size = parent.m_size / grid;
		m_origin = parent.m_origin + childLocation * m_size;
	}

	VecT m_origin;
	VecT m_size;
};

using hct::Vec3d;
using SubdivisionScheme = hct::SimpleSubdivisionScheme<3>;
using Tree = hct::HyperCubeTree< 3, SubdivisionScheme >;
using TreeCursor = TreeCursorWithPosition<Tree>;
std::ostream& operator << (std::ostream& out, Vec3d p) { return p.toStream(out); }

void testTreeCSGRefine(SubdivisionScheme subdivisions)
{
	std::cout << "-----------------------\n";

	Tree tree(subdivisions);
	tree.refine(tree.rootCell());
	size_t nbRootChildren = subdivisions.getLevelSubdivision(0).gridSize();
	for (size_t i = 0; i < nbRootChildren; i++)
	{
		tree.refine(tree.child(tree.rootCell(), i));
	}

	auto sphereA = hct::csg_sphere(Vec3d({ 0.0,0.0,0.0 }), 1.0);
	auto sphereB = hct::csg_sphere(Vec3d({ 0.5,0.5,0.5 }), 0.5);
	auto shape = hct::csg_difference(sphereA, sphereB);

	TreeCursor cursor( subdivisions.getLevelSubdivision(0) );
	std::cout << "Domain bounds = " << cursor.m_size << std::endl;

	Vec3d cellSize = cursor.m_size;
	std::cout << "level 0 : size = " << cellSize << std::endl;
	for (size_t i = 0; i < subdivisions.getNumberOfLevelSubdivisions(); i++)
	{
		cellSize = cellSize / subdivisions.getLevelSubdivision(i);
		std::cout << "level " << (i + 1) << " : size = " << cellSize << std::endl;
	}

	auto T1 = std::chrono::high_resolution_clock::now();

	tree.preorderParseCells(
		[shape, &tree](TreeCursor cursor)
	{
		if (tree.isRefinable(cursor.cell()))
		{
			constexpr size_t nVertices = 2 << TreeCursor::D;
			bool allInside = true;
			bool allOutside = true;
			for (size_t i = 0; i < nVertices; i++)
			{
				auto vertex = hct::bitfield_vec<TreeCursor::D>(i);
				Vec3d p = cursor.m_origin + vertex * cursor.m_size;
				if ( shape(p) > 0.0) { allInside = false; }
				else { allOutside = false; }
			}
			if (!allInside && !allOutside)
			{
				tree.refine(cursor.cell());
			}
		}
	}
	, cursor);

	auto T2 = std::chrono::high_resolution_clock::now();
	auto usec = std::chrono::duration_cast<std::chrono::microseconds>(T2 - T1);

	tree.toStream(std::cout);
	std::cout << "Refinment time = " << usec.count() << " uSec" << std::endl;
}

int main()
{
	{
		SubdivisionScheme subdivisions;
		subdivisions.addLevelSubdivision({ 4,4,20 });
		subdivisions.addLevelSubdivision({ 3,3,3 });
		subdivisions.addLevelSubdivision({ 3,3,3 });
		subdivisions.addLevelSubdivision({ 3,3,3 });
		subdivisions.addLevelSubdivision( {3,3,3} );
		testTreeCSGRefine(subdivisions);
	}

	{
		SubdivisionScheme subdivisions;
		subdivisions.addLevelSubdivision({ 4,4,16 });
		subdivisions.addLevelSubdivision({ 2,2,2 });
		subdivisions.addLevelSubdivision({ 2,2,2 });
		subdivisions.addLevelSubdivision({ 2,2,2 });
		subdivisions.addLevelSubdivision({ 2,2,2 });
		subdivisions.addLevelSubdivision({ 2,2,2 });
		subdivisions.addLevelSubdivision({ 2,2,2 });
		testTreeCSGRefine(subdivisions);
	}

	{
		SubdivisionScheme subdivisions;
		subdivisions.addLevelSubdivision({ 1,2,3 });
		subdivisions.addLevelSubdivision({ 4,5,6 });
		subdivisions.addLevelSubdivision({ 7,8,9 });
		testTreeCSGRefine(subdivisions);
	}

	{
		// TODO: test another subdivision type, like a compile time constant subdivision scheme.
		SubdivisionScheme subdivisions;
		subdivisions.addLevelSubdivision({ 5,5,21 });
		subdivisions.addLevelSubdivision({ 3,3,3 });
		subdivisions.addLevelSubdivision({ 3,3,3 });
		subdivisions.addLevelSubdivision({ 3,3,3 });
		subdivisions.addLevelSubdivision({ 3,3,3 });
		subdivisions.addLevelSubdivision({ 3,3,3 });
		subdivisions.addLevelSubdivision({ 3,3,3 });
		testTreeCSGRefine(subdivisions);
	}

	return 0;
}
