#include "HyperCubeTree.h"
#include "SimpleSubdivisionScheme.h"
#include "GridDimension.h"
#include "HyperCubeTreeVertexOwnershipCursor.h"
#include "HyperCubeTreeDualMesh.h"
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
using TreeDualMesh = hct::HyperCubeTreeDualMesh<Tree>;
using DuallCell = typename TreeDualMesh::DuallCell;

// ==========================================================================
// =============================== test method ==============================
// ==========================================================================

static void testTreeDualMesh(Tree& tree)
{
	static constexpr unsigned int D = Tree::D;

	auto T1 = std::chrono::high_resolution_clock::now();

	size_t nDualCells = 0;
	size_t nDualVertices = 0;
	std::set< hct::HyperCubeTreeCellPosition<D> > dualCellCenters;
	TreeDualMesh::parseDualCells(tree, [&nDualCells,&nDualVertices,&dualCellCenters](DuallCell& dual)
	{
		dualCellCenters.insert(dual.m_center);
		//std::cout << "cell center : "; dual.m_center.toStream(std::cout);
		std::set< hct::HyperCubeTreeCellPosition<D> > dualVertices;
		for (auto& v : dual.m_vertices)
		{
			/*if (v.m_cell.isTreeCell())
			{
				std::cout << " " << v.m_cell;
			}*/
			dualVertices.insert(v.m_coord);
		}
		//std::cout << '\n';
		++ nDualCells;
		nDualVertices += dualVertices.size();
	});

	auto T2 = std::chrono::high_resolution_clock::now();
	auto usec = std::chrono::duration_cast<std::chrono::microseconds>(T2 - T1);

	tree.toStream(std::cout);
	std::cout << "dual cells = " << nDualCells << std::endl;
	std::cout << "unique cell centers = " << dualCellCenters.size() << std::endl;
	std::cout << "average vertices per dual cell = "<< static_cast<double>(nDualVertices)/ nDualCells << std::endl;
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
		testTreeDualMesh(tree);
	}

	{
		std::cout << "\ntest 2 :\n";
		SubdivisionScheme subdivisions;
		subdivisions.addLevelSubdivision({ 3,3,3 });
		Tree tree(subdivisions);
		tree.refine(tree.rootCell());
		testTreeDualMesh(tree);
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
		testTreeDualMesh(tree);
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
		testTreeDualMesh(tree);
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
		testTreeDualMesh(tree);
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
		hct::tree_refine_implicit_surface(tree, shape, subdivisions.getNumberOfLevelSubdivisions() + 1);
		testTreeDualMesh(tree);
	}

	return 0;
}
