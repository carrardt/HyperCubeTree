#include "HyperCubeTree.h"
#include "SimpleSubdivisionScheme.h"
#include "GridDimension.h"
#include "TreeLevelStorage.h"

#include <iostream>

int main()
{
	unsigned int div_4x4x20[3] = { 4, 4, 20 };
	unsigned int div_3x3x3[3] = { 3, 3, 3 };

	hct::SimpleSubdivisionScheme<3> subdivisions;
	subdivisions.addLevelSubdivision(hct::Vec<unsigned int, 3>(div_4x4x20));
	subdivisions.addLevelSubdivision(hct::Vec<unsigned int, 3>(div_3x3x3));
	subdivisions.addLevelSubdivision(hct::Vec<unsigned int, 3>(div_3x3x3));
	subdivisions.addLevelSubdivision(hct::Vec<unsigned int, 3>(div_3x3x3));
	subdivisions.addLevelSubdivision(hct::Vec<unsigned int, 3>(div_3x3x3));

	hct::HyperCubeTree< 3, hct::SimpleSubdivisionScheme<3> > tree(subdivisions);

	hct::TreeLevelArray<double> cellValues;
	tree.addArray(&cellValues);

	return 0;
}
