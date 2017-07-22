#include "HyperCubeTree.h"
#include "SimpleSubdivisionScheme.h"
#include "GridDimension.h"
#include "HyperCubeTreeVertexOwnershipCursor.h"
#include "csg.h"
#include "csg_input.h"
#include "vtkLegacyExport.h"

#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>

using hct::Vec3d;
using SubdivisionScheme = hct::SimpleSubdivisionScheme<3>;
using Tree = hct::HyperCubeTree<3, SubdivisionScheme>;
using HCTVertexOwnershipCursor = hct::HyperCubeTreeVertexOwnershipCursor<Tree>;

std::ostream& operator << (std::ostream& out, Vec3d p) { return p.toStream(out); }

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		std::cout << "Usage : "<<argv[0]<<" input.csg output.vtk" << std::endl;
		return 1;
	}

	SubdivisionScheme subdivisions;
	subdivisions.addLevelSubdivision({ 8,8,8 });
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

	std::ifstream input(argv[1]);
	if (!input)
	{
		std::cerr<<"Error opening file '"<<argv[1]<<"'"<< std::endl;
		return 1;
	}
	std::cout << "read surface from '" << argv[1] << "'" << std::endl;
	auto shape = hct::csg_input<3>(input);

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
				Vec3d p = cursor.position() + vertex;
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
	, HCTVertexOwnershipCursor(tree));
	tree.toStream(std::cout);

	std::ofstream output(argv[2]);
	if (!output)
	{
		std::cerr << "Error opening file '" << argv[2] << "'" << std::endl;
		return 1;
	}
	std::cout<<"output unstructured grid to "<<argv[2] << std::endl;
	hct::vtk::exportUnstructuredGrid(tree, output);

	return 0;
}
