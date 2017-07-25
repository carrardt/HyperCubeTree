#include "HyperCubeTree.h"
#include "SimpleSubdivisionScheme.h"
#include "GridDimension.h"
#include "csg.h"
#include "CellVertexConnectivity.h"

#include <iostream> 
#include <set>

using hct::Vec3d;
std::ostream& operator << (std::ostream& out, Vec3d p) { return p.toStream(out); }
std::ostream& operator << (std::ostream& out, hct::HyperCubeTreeCell c) { return c.toStream(out); }

using SubdivisionScheme = hct::SimpleSubdivisionScheme<3>;
using Tree = hct::HyperCubeTree<3, SubdivisionScheme>;
using HCTVertexOwnershipCursor = hct::HyperCubeTreeVertexOwnershipCursor<Tree>;
using CellVertexConnectivity = hct::CellVertexConnectivity<Tree>;
using CellVertexIds = CellVertexConnectivity::CellVertexIds;
using VertexIdArray = CellVertexConnectivity::VertexIdArray;

std::ostream& operator << (std::ostream& out, const CellVertexIds& cv)
{
	for (size_t i = 0; i < CellVertexConnectivity::CellNumberOfVertices; i++)
	{
		if (i > 0) out << ' ';
		out << cv[i];
	}
	return out;
}

// ==========================================================================
// =============================== test method ==============================
// ==========================================================================

static void testTreeCellConnectivity(Tree& tree)
{
	tree.toStream(std::cout);
	VertexIdArray vertexIds;
	size_t nVertices = 0;
	nVertices = CellVertexConnectivity::compute(tree, vertexIds);
}


// ==========================================================================
// ============================ test main ===================================
// ==========================================================================

int main()
{
	{
		std::cout << "\ntest 1 :\n";
		SubdivisionScheme subdivisions;
		subdivisions.addLevelSubdivision({ 2,2,2 });
		Tree tree(subdivisions);
		tree.refine(tree.rootCell());
		testTreeCellConnectivity(tree);
	}

	{
		std::cout << "\ntest 2 :\n";
		SubdivisionScheme subdivisions;
		subdivisions.addLevelSubdivision({ 3,3,3 });
		Tree tree(subdivisions);
		tree.refine(tree.rootCell());
		testTreeCellConnectivity(tree);
	}

	{
		std::cout << "\ntest 3 :\n";
		SubdivisionScheme subdivisions;
		subdivisions.addLevelSubdivision({ 3,3,3 });
		subdivisions.addLevelSubdivision({ 3,3,3 });
		Tree tree(subdivisions);
		tree.refine(tree.rootCell());
		size_t nbRootChildren = subdivisions.getLevelSubdivision(0).gridSize();
		for (size_t i = 0; i < nbRootChildren; i++)
		{
			tree.refine(tree.child(tree.rootCell(), i));
		}
		testTreeCellConnectivity(tree);
	}

	{
		std::cout << "\ntest 4 :\n";
		SubdivisionScheme subdivisions;
		subdivisions.addLevelSubdivision({ 4,4,20 });
		subdivisions.addLevelSubdivision({ 3,3,3 });
		Tree tree(subdivisions);
		tree.refine(tree.rootCell());
		size_t nbRootChildren = subdivisions.getLevelSubdivision(0).gridSize();
		for (size_t i = 0; i < nbRootChildren; i++)
		{
			tree.refine(tree.child(tree.rootCell(), i));
		}
		testTreeCellConnectivity(tree);
	}

	{
		std::cout << "\ntest 5 :\n";
		SubdivisionScheme subdivisions;
		subdivisions.addLevelSubdivision({ 4,4,20 });
		subdivisions.addLevelSubdivision({ 3,3,3 });
		subdivisions.addLevelSubdivision({ 3,3,3 });
		subdivisions.addLevelSubdivision({ 3,3,3 });
		subdivisions.addLevelSubdivision({ 3,3,3 });
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

		// refine tree given an implicit surface
		tree.preorderParseCells([shape, &tree](const HCTVertexOwnershipCursor& cursor)
		{
			hct::HyperCubeTreeCell cell = cursor.cell();
			if (tree.isRefinable(cell))
			{
				// refine along the implicit surface shape(x)=0
				constexpr size_t CellNumberOfVertices = 1 << Tree::D;
				bool allInside = true;
				bool allOutside = true;
				for (size_t i = 0; i < CellNumberOfVertices; i++)
				{
					auto vertex = hct::bitfield_vec<Tree::D>(i);
					Vec3d p = cursor.position() + vertex ;
					p /= cursor.resolution();
					if (shape(p) > 0.0) { allInside = false; }
					else { allOutside = false; }
				}
				if (!allInside && !allOutside)
				{
					tree.refine(cell);
				}
			}
		}
		, HCTVertexOwnershipCursor(tree) );

		testTreeCellConnectivity(tree);
	}

	return 0;
}
