#include "HyperCubeTree.h"
#include "SimpleSubdivisionScheme.h"
#include "GridDimension.h"
#include "TreeLevelStorage.h"
#include "csg.h"

#include <iostream>
#include <algorithm>
#include <initializer_list>
#include <cmath>

template<unsigned int _D, typename SubdivisionSchemeT>
struct TreeCursorWithPosition : public hct::HyperCubeTree<_D, SubdivisionSchemeT>::HyperCubeTreeCursor
{
	static constexpr unsigned int D = _D;
	using Tree = hct::HyperCubeTree<D, SubdivisionSchemeT>;
	using VecT = hct::Vec<double,D>;
	using SubdivisionGrid = typename Tree::SubdivisionGrid;
	using GridLocation = typename Tree::GridLocation;

	// initialization constructors
	inline TreeCursorWithPosition()
		: m_origin(0.0)
		, m_size(1.0) {}
	inline TreeCursorWithPosition(std::initializer_list<double> l)
		: m_origin(0.0)
		, m_size(l) {}

	// copy constructor
	inline TreeCursorWithPosition(const TreeCursorWithPosition& cursor)
		: Tree::HyperCubeTreeCursor(cursor)
		, m_origin(cursor.m_origin)
		, m_size(cursor.m_size) {}

	// recursion constructor
	inline TreeCursorWithPosition(Tree& tree, TreeCursorWithPosition parent, SubdivisionGrid grid, GridLocation childLocation)
		: Tree::HyperCubeTreeCursor(tree, parent, grid, childLocation)
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
using PositionedTreeCursor = TreeCursorWithPosition<Tree::D, typename Tree::SubdivisionSchemeT>;
std::ostream& operator << (std::ostream& out, Vec3d p) { return p.toStream(out); }


int main()
{
	SubdivisionScheme subdivisions;
	subdivisions.addLevelSubdivision( {4,4,20} );
	subdivisions.addLevelSubdivision( {3,3,3} );
	subdivisions.addLevelSubdivision( {3,3,3} );
	subdivisions.addLevelSubdivision( {3,3,3} );
	//subdivisions.addLevelSubdivision( {3,3,3} );

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

	PositionedTreeCursor cursor = { 4.0 , 4.0 , 20.0 };
	Vec3d cellSize = cursor.m_size;
	std::cout << "level 0 : size = " << cellSize << std::endl;
	for (size_t i = 0; i < subdivisions.getNumberOfLevelSubdivisions(); i++)
	{
		cellSize = cellSize / subdivisions.getLevelSubdivision(i);
		std::cout << "level "<<(i+1)<<" : size = " << cellSize << std::endl;
	}

	tree.preorderParseCells(
		[shape,&tree](PositionedTreeCursor cursor)
		{
			if ( tree.isRefinable(cursor) )
			{
				constexpr size_t nVertices = 2 << PositionedTreeCursor::D;
				bool allInside = true;
				bool allOutside = true;
				for (size_t i = 0; i < nVertices; i++)
				{
					auto vertex = hct::bitfield_vec<3>(i);
					Vec3d p = cursor.m_origin + vertex * cursor.m_size;
					if (shape(p) > 0.0) { allInside = false; }
					else { allOutside = false; }
				}
				if (!allInside && !allOutside)
				{
					tree.refine(cursor);
				}
			}
		}
		, cursor );

	hct::TreeLevelArray<double> cellValues;
	tree.addArray(&cellValues);
	tree.preorderParseCells([&cellValues](HyperCubeTreeCell cell)
		{
			cellValues[cell] = 0.0;
		});	

	tree.toStream(std::cout);

	return 0;
}
