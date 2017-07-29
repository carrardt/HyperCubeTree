#include "csg.h"

#include <iostream>
#include <cmath>
#include <cstdlib>

using hct::Vec3d;
using hct::Vec4d;
std::ostream& operator << (std::ostream& out, Vec3d p) { return p.toStream(out); }

template<typename FuncT>
static inline void testSurfaceFunction(FuncT f, size_t N)
{
	auto interior = hct::csg_inside(f);
	size_t insideCount = 0;
	double outsideDistSum = 0.0;
	double insideDistSum = 0.0;
	Vec3d normalSum(0.0);
	for (size_t i = 0; i < N; i++)
	{
		double x = static_cast<double>(std::rand()) / RAND_MAX;
		double y = static_cast<double>(std::rand()) / RAND_MAX;
		double z = static_cast<double>(std::rand()) / RAND_MAX;
		Vec3d p = { x,y,z };
		Vec4d plane = f(p);
		double dist = plane.val;
		normalSum += plane;
		if (dist > 0.0) { outsideDistSum += dist; }
		else { insideDistSum += dist;}
		bool inside = interior(p);
		if (inside)	{ ++insideCount; }
	}
	double volume = static_cast<double>(insideCount) / static_cast<double>(N);
	std::cout << "inside ratio = " << volume << std::endl;
	std::cout << "average positive surface distance = " << outsideDistSum / N << std::endl;
	std::cout << "average negative surface distance = " << insideDistSum / N << std::endl;
	std::cout << "average normal = " << normalSum/N << std::endl;
}

int main(int argc, char* argv[])
{
	size_t N = 1000;
	if (argc >= 2)
	{
		N = std::atoi(argv[1]);
	}
	if (argc >= 3)
	{
		std::srand(std::atoi(argv[2]));
	}

	// quarter sphere
	{
		std::cout << "1/8 sphere in the unit cube. Volume=0.523" << std::endl;
		auto unitSphere = hct::csg_sphere(Vec3d({ 0.0,0.0,0.0 }), 1.0);
		testSurfaceFunction(unitSphere, N);
	}

	{
		auto smallSphere = hct::csg_sphere(Vec3d({ 0.25,0.25,0.25 }), 0.25);
		std::cout << "Sphere with radius=0.25, entierely inside the unit cube.Volume=0.0654" << std::endl;
		testSurfaceFunction(smallSphere, N);
	}

	{
		auto sphereA = hct::csg_sphere(Vec3d({ 0.0,0.0,0.0 }), 1.0);
		auto sphereB = hct::csg_sphere(Vec3d({ 0.25,0.25,0.25 }), 0.25);
		auto sphereWithHole = hct::csg_intersection(sphereA, hct::csg_negate(sphereB) );
		std::cout << "subtraction of the 2 previous psheres. Volume=0.4581" << std::endl;
		testSurfaceFunction(sphereWithHole, N);
	}

	{
		auto sphereA = hct::csg_sphere(Vec3d({ 0.0,0.0,0.0 }), 1.0);
		auto sphereB = hct::csg_sphere(Vec3d({ 0.5,0.5,0.5 }), 0.5);
		auto deathStar = hct::csg_difference(sphereA, sphereB);
		std::cout << "Death star. Volume unknown" << std::endl;
		testSurfaceFunction(deathStar, N);
	}

	return 0;
}
