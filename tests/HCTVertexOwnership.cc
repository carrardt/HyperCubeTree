#include "HyperCubeTree.h"
#include "SimpleSubdivisionScheme.h"
#include "GridDimension.h"
#include "csg.h"
#include "HyperCubeTreeVertexOwnershipCursor.h"

#include <iostream> 
#include <set>

using hct::Vec3d;
std::ostream& operator << (std::ostream& out, Vec3d p) { return p.toStream(out); }
std::ostream& operator << (std::ostream& out, hct::HyperCubeTreeCell c) { return c.toStream(out); }

using SubdivisionScheme = hct::SimpleSubdivisionScheme<3>;
using Tree = hct::HyperCubeTree<3, SubdivisionScheme>;
using HCTVertexOwnershipCursor = hct::HyperCubeTreeVertexOwnershipCursor<Tree>;

// ==========================================================================
// =============================== test method ==============================
// ==========================================================================

static void testTreeVertexOwnership(Tree& tree)
{
	tree.toStream(std::cout);

	size_t nbOwnedVertices = 0;
	std::set<hct::Vec3d, decltype(&hct::Vec3d::less_operator)> posSet(&hct::Vec3d::less_operator);

	tree.preorderParseCells([&nbOwnedVertices,&posSet](const HCTVertexOwnershipCursor& cursor)
	{
		using HCubeComponentValue = typename HCTVertexOwnershipCursor::HCubeComponentValue;
		constexpr size_t CellNumberOfVertices = 1 << Tree::D;
		for (size_t i = 0; i < CellNumberOfVertices; i++)
		{
			auto vertex = hct::bitfield_vec<Tree::D>(i);
			Vec3d p = cursor.position() + vertex;
			p /= cursor.resolution();
			posSet.insert(p);
			if (cursor.ownsVertex(i))
			{
				++nbOwnedVertices;
			}
		}
	}
	, HCTVertexOwnershipCursor(tree) );

	std::cout << "different vertices = " << posSet.size() << std::endl;
	std::cout << "owned vertices = " << nbOwnedVertices << std::endl;

	assert(nbOwnedVertices == posSet.size());

	/*if (posSet.size() < 100)
	{
		for (Vec3d p : posSet)
		{
			std::cout << p << std::endl;
		}
	}*/
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
		testTreeVertexOwnership(tree);
	}

	{
		std::cout << "\ntest 2 :\n";
		SubdivisionScheme subdivisions;
		subdivisions.addLevelSubdivision({ 3,3,3 });
		Tree tree(subdivisions);
		tree.refine(tree.rootCell());
		testTreeVertexOwnership(tree);
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
		testTreeVertexOwnership(tree);
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
		testTreeVertexOwnership(tree);
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

		Vec3d domainUnitSize(1.0);

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

		testTreeVertexOwnership(tree);
	}

	return 0;
}
