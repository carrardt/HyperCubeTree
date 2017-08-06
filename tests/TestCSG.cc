#include "csg.h"

#include <iostream>
#include <cmath>
#include <cstdlib>

using hct::Vec3d;
using hct::Vec4d;
std::ostream& operator << (std::ostream& out, Vec3d p) { return p.toStream(out); }

std::ostream& operator << (std::ostream& out, hct::ScalarFunctionValue<3> fval)
{
	out << "f="<< fval.m_value<<",grad="<<fval.m_gradient;
	return out;
}


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
		auto Fp = f(p);
		double dist = Fp.m_value;
		normalSum += Fp.m_gradient;
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

template<typename FuncT>
static inline void basicFunctionTest(FuncT f)
{
	std::cout << "f(0, 0, 0) = { " << f(Vec3d({ 0,0,0 })) << " }\n";
	std::cout << "f(1, 0, 0) = { " << f(Vec3d({ 1,0,0 })) << " }\n";
	std::cout << "f(0, 1, 0) = { " << f(Vec3d({ 0,1,0 })) << " }\n";
	std::cout << "f(0, 0, 1) = { " << f(Vec3d({ 0,0,1 })) << " }\n";
	std::cout << "f(1, 1, 1) = { " << f(Vec3d({ 1,1,1 })) << " }\n";
	std::cout << "f(-1, -1, -1) = { " << f(Vec3d({ -1,-1,-1 })) << " }\n";
}

int main(int argc, char* argv[])
{
	size_t N = 10000;
	if (argc >= 2)
	{
		N = std::atoi(argv[1]);
	}
	if (argc >= 3)
	{
		std::srand(std::atoi(argv[2]));
	}

	// 1st : sample tests
	std::cout << "\nf=distance(0,0,0) :\n";
	basicFunctionTest(hct::point_distance_function(Vec3d({0,0,0})));
	std::cout << "\nf=distance(1,1,1) :\n";
	basicFunctionTest(hct::point_distance_function(Vec3d({1,1,1})));
	std::cout << "\nf=distance(0.5,0.5,0.5) :\n";
	basicFunctionTest(hct::point_distance_function(Vec3d({ 0.5,0.5,0.5 })));
	std::cout << "\nf=constant(3.14) :\n";
	basicFunctionTest(hct::ConstantFunction<3>(3.14));
	std::cout << "\nf=-constant(3.14) :\n";
	basicFunctionTest( hct::negate_function( hct::ConstantFunction<3>(3.14) ) );
	std::cout << "\nf=constant(3.14)+constant(0.86) :\n";
	basicFunctionTest(hct::add_function(hct::ConstantFunction<3>(3.14), hct::ConstantFunction<3>(0.86))  );

	std::cout << "\nf=distance(0.5,0.5,0.5)-constant(1.0) :\n";
	basicFunctionTest( hct::add_function(hct::point_distance_function(Vec3d({ 0.5,0.5,0.5 })) , negate_function(hct::ConstantFunction<3>(3.14)) )   );

	// volume tests
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
		auto sphereWithHole = hct::csg_difference(sphereA, sphereB );
		std::cout << "subtraction of the 2 previous spheres. Volume=0.4581" << std::endl;
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
