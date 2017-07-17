#include "HyperCubeTree.h"
#include "SimpleSubdivisionScheme.h"
#include "GridDimension.h"
#include "csg.h"
#include "HyperCubeTreeLocatedCursor.h"
#include "HyperCubeTreeNbhCursor.h"

#include <iostream>

using hct::Vec3d;
std::ostream& operator << (std::ostream& out, Vec3d p) { return p.toStream(out); }

int main()
{
	using SubdivisionScheme = hct::SimpleSubdivisionScheme<3>;
	using Tree = hct::HyperCubeTree< 3, SubdivisionScheme >;
	using TreeCursor = hct::HyperCubeTreeLocatedCursor<Tree>;
	using NbhTreeCursor = hct::HyperCubeTreeNbhCursor<Tree>;

	SubdivisionScheme subdivisions;
	subdivisions.addLevelSubdivision({ 4,4,20 });
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

	TreeCursor cursor(subdivisions.getLevelSubdivision(0));
	std::cout << "Domain bounds = " << cursor.m_size << std::endl;

	Vec3d cellSize = cursor.m_size;
	std::cout << "level 0 : size = " << cellSize << std::endl;
	for (size_t i = 0; i < subdivisions.getNumberOfLevelSubdivisions(); i++)
	{
		cellSize = cellSize / subdivisions.getLevelSubdivision(i);
		std::cout << "level " << (i + 1) << " : size = " << cellSize << std::endl;
	}

	tree.preorderParseCells( [shape, &tree](TreeCursor cursor)
	{
		hct::HyperCubeTreeCell cell = cursor.cell();
		// refine along the implicit surface shape(x)=0
		if (tree.isRefinable(cell))
		{
			constexpr size_t nVertices = 2 << TreeCursor::D;
			bool allInside = true;
			bool allOutside = true;
			for (size_t i = 0; i < nVertices; i++)
			{
				auto vertex = hct::bitfield_vec<TreeCursor::D>(i);
				Vec3d p = cursor.m_origin + vertex * cursor.m_size;
				if (shape(p) > 0.0) { allInside = false; }
				else { allOutside = false; }
			}
			if (!allInside && !allOutside)
			{
				tree.refine(cell);
			}
		}
	}
	, cursor);
	tree.toStream(std::cout);


	std::cout << "\nTree traversal with Neighborhood :\n";
	NbhTreeCursor nbhCursor;
	tree.preorderParseCells( [](NbhTreeCursor nbhCursor)
	{
		nbhCursor.cell().toStream(std::cout);
		std::cout << '\n';
	}
	, nbhCursor);

	return 0;
}
