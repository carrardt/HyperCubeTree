#include "HyperCubeTree.h"
#include "SimpleSubdivisionScheme.h"
#include "GridDimension.h"
#include "csg.h"
#include "HyperCubeTreeVertexOwnershipCursor.h"

#include <iostream> 
#include <set>
#include <chrono>

using hct::Vec3d;
std::ostream& operator << (std::ostream& out, Vec3d p) { return p.toStream(out); }
std::ostream& operator << (std::ostream& out, hct::HyperCubeTreeCell c) { return c.toStream(out); }

using SubdivisionScheme = hct::SimpleSubdivisionScheme<3>;
using Tree = hct::HyperCubeTree<3, SubdivisionScheme>;
using HCTVertexOwnershipCursor = hct::HyperCubeTreeVertexOwnershipCursor<Tree>;

// ==========================================================================
// =============================== test method ==============================
// ==========================================================================

static size_t testTreeVertexOwnership(Tree& tree)
{
	tree.toStream(std::cout);

	size_t nbOwnedVertices = 0;
	std::set<hct::Vec3d> posSet;

	auto T1 = std::chrono::high_resolution_clock::now();

	tree.preorderParseCells([&tree,&nbOwnedVertices,&posSet](const HCTVertexOwnershipCursor& cursor)
	{
		using HCubeComponentValue = typename HCTVertexOwnershipCursor::HCubeComponentValue;
		constexpr size_t CellNumberOfVertices = 1 << Tree::D;
		for (size_t i = 0; i < CellNumberOfVertices; i++)
		{
			auto vertex = hct::bitfield_vec<Tree::D>(i);
			Vec3d p = ( cursor.position() + vertex ).normalize();
			posSet.insert(p);
			if (cursor.ownsVertex(i))
			{
				// asertion must be true, according to statement 1. in HyperCubeTreeVertexOwnershipCursor.h
				assert( tree.isLeaf(cursor.cell()) );
				++nbOwnedVertices;
			}
		}
	}
	, HCTVertexOwnershipCursor(tree) );

	auto T2 = std::chrono::high_resolution_clock::now();
	auto usec = std::chrono::duration_cast<std::chrono::microseconds>(T2 - T1);

	std::cout << "different vertices = " << posSet.size() << std::endl;
	std::cout << "owned vertices = " << nbOwnedVertices << std::endl;
	std::cout << "time = " << usec.count() << "uS" << std::endl;

	// all vertices must be owned, they must be owned by exactly one cell
	assert(nbOwnedVertices == posSet.size());

	return nbOwnedVertices;
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
		assert( testTreeVertexOwnership(tree) == 27 );
	}

	{
		std::cout << "\ntest 2 :\n";
		SubdivisionScheme subdivisions;
		subdivisions.addLevelSubdivision({ 3,3,3 });
		Tree tree(subdivisions);
		tree.refine(tree.rootCell());
		assert( testTreeVertexOwnership(tree) == 64);
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
		assert( testTreeVertexOwnership(tree) == 1000 );
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
		assert( testTreeVertexOwnership(tree) == 10309 );
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
					Vec3d p = ( cursor.position() + vertex ).normalize();
					if (shape(p).val > 0.0) { allInside = false; }
					else { allOutside = false; }
				}
				if (!allInside && !allOutside)
				{
					tree.refine(cell);
				}
			}
		}
		, HCTVertexOwnershipCursor(tree) );

		assert( testTreeVertexOwnership(tree) == 5292121);
	}

	return 0;
}
