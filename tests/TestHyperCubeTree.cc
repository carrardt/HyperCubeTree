#include "HyperCubeTree.h"
#include "SimpleSubdivisionScheme.h"
#include "GridDimension.h"
#include "TreeLevelStorage.h"

#include <iostream>
#include <algorithm>
#include <initializer_list>
#include <cmath>

int main()
{
	hct::SimpleSubdivisionScheme<3> subdivisions;
	subdivisions.addLevelSubdivision( {4,4,20} );
	subdivisions.addLevelSubdivision( {3,3,3} );
	subdivisions.addLevelSubdivision( {3,3,3} );
	subdivisions.addLevelSubdivision( {3,3,3} );
	subdivisions.addLevelSubdivision( {3,3,3} );

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

	std::cout << "initialize cellValues" << std::endl;
	tree.preorderParseCells([&cellValues](HyperCubeTreeCell cell) { cellValues[cell] = cell.m_level*1000000.0 + cell.m_index; });

	double maxval = 0.0;
	std::cout << "read cellValues" << std::endl;
	tree.preorderParseCells([&maxval,&cellValues](HyperCubeTreeCell cell) { maxval = std::max(maxval,cellValues[cell]); });
	std::cout << "max value = " << maxval << std::endl;

	return 0;
}
