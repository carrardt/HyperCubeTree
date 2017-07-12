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

	tree.toStream(std::cout);

	// rafiner la racine
	std::cout << "subdivide root\n";
	tree.refine(tree.rootCell());

	tree.toStream(std::cout);

	// rafiner entierement le premier niveau après la racine
	std::cout << "subdivide 2nd level cells\n";
	size_t nbRootChildren = subdivisions.getLevelSubdivision(0).gridSize();
	for (size_t i = 0; i < nbRootChildren; i++)
	{
		tree.refine(tree.child(tree.rootCell(),i));
	}
	tree.toStream(std::cout);

	return 0;
}
