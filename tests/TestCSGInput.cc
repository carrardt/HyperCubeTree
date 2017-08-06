
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>

#include "ScalarFunction.h"
#include "ScalarFunctionInput.h"

using hct::Vec3d;
std::ostream& operator << (std::ostream& out, Vec3d p) { return p.toStream(out); }

template<typename FuncT>
static inline void testSurfaceFunction(FuncT f, size_t N)
{
	auto interior = hct::csg_inside(f);
	size_t insideCount = 0;
	double outsideDistSum = 0.0;
	double insideDistSum = 0.0;
	for (size_t i = 0; i < N; i++)
	{
		double x = static_cast<double>(std::rand()) / RAND_MAX;
		double y = static_cast<double>(std::rand()) / RAND_MAX;
		double z = static_cast<double>(std::rand()) / RAND_MAX;
		Vec3d p = { x,y,z };
		double dist = f(p).value();
		if (dist > 0.0) { outsideDistSum += dist; }
		else { insideDistSum += dist;}
		bool inside = interior(p);
		if (inside)	{ ++insideCount; }
	}
	double volume = static_cast<double>(insideCount) / static_cast<double>(N);
	std::cout << "inside ratio = " << volume << std::endl;
	std::cout << "average positive surface distance = " << outsideDistSum / N << std::endl;
	std::cout << "average negative surface distance = " << insideDistSum / N << std::endl;
}

int main(int argc, char* argv[])
{
	if (argc > 1)
	{
		std::cout << "read surface from " << argv[1] << std::endl;
		std::ifstream fin(argv[1]);
		auto surf = hct::scalar_function_read<3,double>(fin);
		testSurfaceFunction(surf, 10000);
	}
	else
	{
		std::cout << "read surface from standard input"<< std::endl;
		auto surf = hct::scalar_function_read<3, double>(std::cin);
		testSurfaceFunction(surf, 10000);
	}

	return 0;
}
