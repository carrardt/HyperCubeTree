#include "HyperCubeTree.h"
#include "SimpleSubdivisionScheme.h"
#include "GridDimension.h"
#include "CellVertexConnectivity.h"
#include "ScalarFunction.h"
#include "TreeRefineImplicitSurface.h"

#include <iostream> 
#include <set>
#include <chrono>

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
	static constexpr size_t CellNumberOfVertices = CellVertexConnectivity::CellNumberOfVertices;
	static constexpr unsigned int D = Tree::D;
	VertexIdArray vertexIds;

	auto T1 = std::chrono::high_resolution_clock::now();

	// build connectivity
	size_t nVertices = CellVertexConnectivity::compute(tree, vertexIds);

	// verify that all values are set
	size_t totalVertices = 0;
	tree.postorderParseCells([nVertices,&totalVertices,&vertexIds](const typename Tree::DefaultTreeCursor& cursor)
	{
		hct::HyperCubeTreeCell cell = cursor.cell();
		for (size_t i = 0; i < CellNumberOfVertices; i++)
		{
			++totalVertices;
			assert(vertexIds[cell][i] >= 0 && vertexIds[cell][i] < nVertices);
		}
	});

	auto T2 = std::chrono::high_resolution_clock::now();
	auto usec = std::chrono::duration_cast<std::chrono::microseconds>(T2 - T1);

	tree.toStream(std::cout);
	std::cout << "totalVertices=" << totalVertices << ", nVertices=" << nVertices << ", time="<< usec.count() <<"uS" << std::endl;
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
		hct::tree_refine_implicit_surface(tree, shape, subdivisions.getNumberOfLevelSubdivisions() + 1);
		testTreeCellConnectivity(tree);
	}

	{
		std::cout << "\ntest 6 :\n";
		SubdivisionScheme subdivisions;
		subdivisions.addLevelSubdivision({ 3,4,5 });
		subdivisions.addLevelSubdivision({ 4,3,2 });
		subdivisions.addLevelSubdivision({ 3,2,1 });
		subdivisions.addLevelSubdivision({ 4,4,4 });
		subdivisions.addLevelSubdivision({ 5,5,5 });
		subdivisions.addLevelSubdivision({ 2,2,2 });
		Tree tree(subdivisions);
		tree.refine(tree.rootCell());
		size_t nbRootChildren = subdivisions.getLevelSubdivision(0).gridSize();
		for (size_t i = 0; i < nbRootChildren; i++)
		{
			tree.refine(tree.child(tree.rootCell(), i));
		}

		auto sphereA = hct::csg_sphere(Vec3d({ 0.0,0.0,0.0 }), 1.0);
		auto sphereB = hct::csg_sphere(Vec3d({ 0.5,0.5,0.5 }), 0.5);
		auto sphereC = hct::csg_sphere(Vec3d({ 0.75,0.75,0.5 }), 0.25);
		auto shape = hct::csg_union( hct::csg_difference(sphereA, sphereB) , sphereC);

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
					Vec3d p = ( cursor.position() + vertex ).normalize();
					if (shape(p).value() > 0.0) { allInside = false; }
					else { allOutside = false; }
				}
				if (!allInside && !allOutside)
				{
					tree.refine(cell);
				}
			}
		}
		, HCTVertexOwnershipCursor(tree));

		testTreeCellConnectivity(tree);
	}


	return 0;
}
