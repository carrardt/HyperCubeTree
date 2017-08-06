#include "HyperCubeTree.h"
#include "SimpleSubdivisionScheme.h"
#include "GridDimension.h"
#include "HyperCubeTreeVertexOwnershipCursor.h"
#include "HyperCubeTreeLocatedCursor.h"
#include "ScalarFunction.h"
#include "ScalarFunctionInput.h"
#include "vtkLegacyExportDual.h"

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
	std::string subdivisionFileName = std::string(HCT_DATA_DIR) + "/levels_8x8x8_3x3x3_x2.div";
	std::string csgFileName = std::string(HCT_DATA_DIR) + "/deathstar.csg";
	std::string outputFileName = "output.vtk";

	if (argc >= 2) { subdivisionFileName = argv[1]; }
	if (argc >= 3) { csgFileName = argv[2]; }
	if (argc >= 4) { outputFileName = argv[3]; }


	std::ifstream levels_input(subdivisionFileName);
	if (!levels_input)
	{
		std::cerr << "Error opening file '" << subdivisionFileName << "'" << std::endl;
		return 1;
	}
	std::cout << "read levels from " << subdivisionFileName << '\n';
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

	std::ifstream input(csgFileName);
	if (!input)
	{
		std::cerr<<"Error opening file '"<< csgFileName <<"'"<< std::endl;
		return 1;
	}
	std::cout << "read CSG from '" << csgFileName << "'" << std::endl;
	std::cout.flush();
	auto shape = hct::scalar_function_read<3, double>(input);

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
		Vec3d p = cursor.position().addHalfUnit().normalize();
		auto Fp = shape(p);
		cellSurfaceDistance[cell] = Fp.value();
		cellSurfaceNormal[cell] = Fp.gradient();
	}
	, LocatedTreeCursor() );

	tree.toStream(std::cout);

	std::ofstream output(outputFileName);
	if (!output)
	{
		std::cerr << "Error opening file '" << outputFileName << "'" << std::endl;
		return 1;
	}
	std::cout<<"output dual unstructured grid to "<< outputFileName << std::endl;
	hct::vtk::exportDualUnstructuredGrid(tree, output);

	return 0;
}
