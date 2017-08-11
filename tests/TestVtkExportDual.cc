#include "HyperCubeTree.h"
#include "SimpleSubdivisionScheme.h"
#include "HyperCubeTreeInput.h"
#include "vtkLegacyExportDual.h"

#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <vector>

using hct::Vec3d;
using hct::Vec4d;
using SubdivisionScheme = hct::SimpleSubdivisionScheme<3>;
using Tree = hct::HyperCubeTree<3, SubdivisionScheme>;

// this allows vectors to be output with space separators
std::ostream& operator << (std::ostream& out, Vec3d p)
{
	return p.toStream(out," ");
}

int main(int argc, char* argv[])
{
	std::string inputFileName = std::string(HCT_DATA_DIR) + "/deathstar3d_3levels.hct";
	std::string outputFileName = "output.vtk";

	if (argc >= 2) { inputFileName = argv[1]; }
	if (argc >= 3) { outputFileName = argv[2]; }

	std::ifstream input(inputFileName);
	if (!input)
	{
		std::cerr << "Error opening file '" << inputFileName << "'" << std::endl;
		return 1;
	}

	std::cout << "read tree from " << inputFileName << std::endl;
	std::cout.flush();
	std::vector< hct::TreeLevelArray<double> * > scalars;
	std::vector< hct::TreeLevelArray<hct::Vec<double, 3> > * > vectors;
	Tree* tree = hct::read_tree<3,double>(input,scalars,vectors);

	assert(tree->checkArraySizes());
	tree->toStream(std::cout);

	std::ofstream output(outputFileName);
	if (!output)
	{
		std::cerr << "Error opening file '" << outputFileName << "'" << std::endl;
		return 1;
	}
	std::cout<<"output dual unstructured grid to "<< outputFileName << std::endl;
	hct::vtk::exportDualUnstructuredGrid(*tree, output);

	return 0;
}
