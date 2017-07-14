#include "HyperCubeTree.h"
#include "SimpleSubdivisionScheme.h"
#include "GridDimension.h"
#include "TreeLevelStorage.h"
#include "csg.h"

#include <iostream>
#include <algorithm>
#include <initializer_list>
#include <cmath>

using hct::Vec3d;
std::ostream& operator << (std::ostream& out, Vec3d p) { return p.toStream(out); }

template<typename FuncT>
static inline void testSurfaceFunction(FuncT f, Vec3d p)
{
	std::cout << p << " -> " << f(p) << std::endl;
}

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

	// CSG construct of a death-star like shape
	auto sphere1 = hct::csg_sphere( { 0.5,0.5,0.5 } , 0.5 );
	auto sphere2 = hct::csg_sphere({ 0.75,0.75,0.75 } , -0.25 );
	auto deathStar = hct::csg_intersection( sphere1, sphere2);
	auto interior = hct::csg_inside(deathStar);

	/*
	std::cout << "surface function : \n";
	testSurfaceFunction(deathStar, Vec3d({ 0.5,0.5,0.5 }));
	testSurfaceFunction(deathStar, Vec3d({ 1.0,1.0,1.0 }));
	testSurfaceFunction(deathStar, Vec3d({ 0.9,0.9,0.9 }));
	testSurfaceFunction(deathStar, Vec3d({ 0.8,0.8,0.8 }));
	testSurfaceFunction(deathStar, Vec3d({ 0.7,0.7,0.7 }));
	testSurfaceFunction(deathStar, Vec3d({ 0.6,0.6,0.6 }));
	testSurfaceFunction(deathStar, Vec3d({ 0.9,0.5,0.9 }));
	testSurfaceFunction(deathStar, Vec3d({ 0.8,0.5,0.8 }));
	testSurfaceFunction(deathStar, Vec3d({ 0.7,0.5,0.7 }));
	testSurfaceFunction(deathStar, Vec3d({ 0.6,0.5,0.6 }));
	*/

	return 0;
}
