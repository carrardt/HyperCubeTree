#include "HyperCubeTree.h"
#include "SimpleSubdivisionScheme.h"
#include "GridDimension.h"
#include "HyperCubeTreeVertexOwnershipCursor.h"
#include "HyperCubeTreeLocatedCursor.h"
#include "csg.h"
#include "csg_input.h"
#include "vtkLegacyExport.h"

#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>

using hct::Vec3d;
using hct::Vec4d;
using SubdivisionScheme = hct::SimpleSubdivisionScheme<3>;
using Tree = hct::HyperCubeTree<3, SubdivisionScheme>;
using HCTVertexOwnershipCursor = hct::HyperCubeTreeVertexOwnershipCursor<Tree>;
using LocatedTreeCursor = hct::HyperCubeTreeLocatedCursor<Tree>;

// this allows vectors to be output with space separators
std::ostream& operator << (std::ostream& out, Vec3d p)
{
	return p.toStream(out," ");
}

int main(int argc, char* argv[])
{
	if (argc < 4)
	{
		std::cout << "Usage : "<<argv[0]<<" levels.div input.csg output.vtk" << std::endl;
		return 1;
	}

	std::ifstream levels_input(argv[1]);
	if (!levels_input)
	{
		std::cerr << "Error opening file '" << argv[1] << "'" << std::endl;
		return 1;
	}
	std::cout << "read levels from " << argv[1] << '\n';
	std::cout.flush();
	SubdivisionScheme subdivisions;
	subdivisions.fromStream(levels_input);

	Tree tree(subdivisions);
	tree.refine(tree.rootCell());
	size_t nbRootChildren = subdivisions.getLevelSubdivision(0).gridSize();
	for (size_t i = 0; i < nbRootChildren; i++)
	{
		tree.refine(tree.child(tree.rootCell(), i));
	}

	std::ifstream input(argv[2]);
	if (!input)
	{
		std::cerr<<"Error opening file '"<<argv[2]<<"'"<< std::endl;
		return 1;
	}
	std::cout << "read CSG from '" << argv[2] << "'" << std::endl;
	std::cout.flush();
	auto shape = hct::csg_input<3>(input);

	// refine tree given an implicit surface
	std::cout << "build tree\n";
	std::cout.flush();

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
				Vec3d p = (cursor.position() + vertex).normalize();
				if (shape(p).val > 0.0) { allInside = false; }
				else { allOutside = false; }
			}
			if (!allInside && !allOutside)
			{
				tree.refine(cell);
			}
		}
	}
	, HCTVertexOwnershipCursor(tree));

	// compute a scalar value derived from csg surface
	hct::TreeLevelArray<double> cellSurfaceDistance;
	cellSurfaceDistance.setName("surf_dist");
	tree.addArray(&cellSurfaceDistance);
	cellSurfaceDistance.fill(1000.0);

	hct::TreeLevelArray< Vec3d > cellSurfaceNormal;
	cellSurfaceNormal.setName("surf_normal");
	tree.addArray(&cellSurfaceNormal);
	cellSurfaceNormal.fill( Vec3d(0.0) );

	tree.parseLeaves([shape,&cellSurfaceDistance,&cellSurfaceNormal](const LocatedTreeCursor& cursor)
	{
		hct::HyperCubeTreeCell cell = cursor.cell();
		Vec3d p = cursor.m_origin + ( cursor.m_size * 0.5 );
		Vec4d plane = shape(p);
		Vec3d normal(plane);
		cellSurfaceDistance[cell] = plane.val;
		cellSurfaceNormal[cell] = normal;
	}
	, LocatedTreeCursor(Vec3d(1.0)) );

	tree.toStream(std::cout);

	std::ofstream output(argv[3]);
	if (!output)
	{
		std::cerr << "Error opening file '" << argv[3] << "'" << std::endl;
		return 1;
	}
	std::cout<<"output unstructured grid to "<<argv[3] << std::endl;
	hct::vtk::exportUnstructuredGrid(tree, output);

	return 0;
}
