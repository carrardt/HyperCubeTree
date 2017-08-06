#include "HyperCubeTree.h"
#include "HyperCubeTreeLocatedCursor.h"
#include "SimpleSubdivisionScheme.h"
#include "StaticSubdivisionScheme.h"
#include "GridDimension.h"
#include "csg.h"

#include <iostream>
#include <algorithm>
#include <initializer_list>
#include <cmath>
#include <chrono>

using hct::Vec3d;
std::ostream& operator << (std::ostream& out, Vec3d p) { return p.toStream(out); }

template<typename SubdivisionSchemeT>
static void testTreeCSGRefine(SubdivisionSchemeT subdivisions)
{
	using Tree = hct::HyperCubeTree< 3, SubdivisionSchemeT >;
	using TreeCursor = hct::HyperCubeTreeLocatedCursor<Tree>;

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

	Vec3d cellSize = Vec3d(1.0);
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
			constexpr size_t nVertices = 1 << TreeCursor::D;
			bool allInside = true;
			bool allOutside = true;
			Vec3d normal;
			bool sameDirection = true;
			for (size_t i = 0; i < nVertices; i++)
			{
				Vec3d x = cursor.vertexPosition(i).normalize();
				auto Fx = shape(x);
				if (allInside && allOutside)
				{
					normal = Fx.gradient();
				}
				else if( normal.dot( Fx.gradient() ) < 0.0)
				{
					sameDirection = false;
				}
				if ( Fx.value() > 0.0) { allInside = false; }
				else { allOutside = false; }
			}
			if ( (!allInside && !allOutside) || !sameDirection)
			{
				tree.refine(cursor.cell());
			}
		}
	}
	, TreeCursor() );

	auto T2 = std::chrono::high_resolution_clock::now();
	auto usec = std::chrono::duration_cast<std::chrono::microseconds>(T2 - T1);

	tree.toStream(std::cout);
	std::cout << "Refinment time = " << usec.count() << " uSec" << std::endl;
}

int main()
{
	{
		hct::SimpleSubdivisionScheme<3> subdivisions;
		subdivisions.addLevelSubdivision({ 4,4,20 });
		subdivisions.addLevelSubdivision({ 3,3,3 });
		subdivisions.addLevelSubdivision({ 3,3,3 });
		subdivisions.addLevelSubdivision({ 3,3,3 });
		subdivisions.addLevelSubdivision( {3,3,3} );
		testTreeCSGRefine(subdivisions);
	}

	{
		hct::SimpleSubdivisionScheme<3> subdivisions;
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
		hct::SimpleSubdivisionScheme<3> subdivisions;
		subdivisions.addLevelSubdivision({ 1,2,3 });
		subdivisions.addLevelSubdivision({ 4,5,6 });
		subdivisions.addLevelSubdivision({ 7,8,9 });
		testTreeCSGRefine(subdivisions);
	}

	{
		hct::SimpleSubdivisionScheme<3> subdivisions;
		subdivisions.addLevelSubdivision({ 5,5,21 });
		subdivisions.addLevelSubdivision({ 3,3,3 });
		subdivisions.addLevelSubdivision({ 3,3,3 });
		subdivisions.addLevelSubdivision({ 3,3,3 });
		subdivisions.addLevelSubdivision({ 3,3,3 });
		subdivisions.addLevelSubdivision({ 3,3,3 });
		subdivisions.addLevelSubdivision({ 3,3,3 });
		testTreeCSGRefine(subdivisions);
	}

	{
		using InitialSubdiv = hct::StaticSubdivisionScheme< 1,   5, 5, 21 >;
		using Subdiv = hct::StaticSubdivisionScheme< 6,   3, 3, 3 >;
		hct::StaticSubdivisionSchemeCombo< InitialSubdiv, Subdiv > staticSubdivCheme;
		testTreeCSGRefine(staticSubdivCheme);
	}

	return 0;
}
