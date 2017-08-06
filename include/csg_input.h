#pragma once

#include "csg.h"
#include <string>
#include <memory>
#include <vector>
#include <cstdlib>

namespace hct
{

	template<unsigned int D, typename StreamT>
	ScalarFunctionDelegate<D,double> csg_input(StreamT& input)
	{
		std::vector< ScalarFunctionDelegate<D,double> > stack;
		bool endInput = false;
		while (input && !endInput)
		{
			std::string token;
			input >> token;
			if (token == "sphere")
			{
				double center[D];
				for (size_t i = 0; i < D; i++) { center[i] = 0.0;  input >> center[i]; }
				double radius = 0;
				input >> radius;
				auto surf = csg_sphere(Vec<double,D>(center), radius);
				stack.push_back( scalar_function_delegate(surf) );
			}
			else if (token == "plane")
			{
				double normal[D];
				for (size_t i = 0; i < D; i++) { normal[i] = 0.0;  input >> normal[i]; }
				double offset = 0;
				input >> offset;
				auto surf = plane_function(Vec<double, D + 1>(offset, Vec<double, D>(normal)));
				stack.push_back(scalar_function_delegate(surf));
			}
			else if (token == "union")
			{
				auto a = stack.back(); stack.pop_back();
				auto b = stack.back(); stack.pop_back();
				auto surf = csg_union(a, b);
				stack.push_back(scalar_function_delegate(surf));
			}
			else if (token == "intersection")
			{
				auto a = stack.back(); stack.pop_back();
				auto b = stack.back(); stack.pop_back();
				auto surf = csg_intersection(a, b);
				stack.push_back(scalar_function_delegate(surf));
			}
			else if (token == "difference")
			{
				auto a = stack.back(); stack.pop_back();
				auto b = stack.back(); stack.pop_back();
				auto surf = csg_difference(a, b);
				stack.push_back(scalar_function_delegate(surf));
			}
			else if (token == "end")
			{
				endInput = true;
			}
			else
			{
				std::abort();
			}
		}
		return stack.back();
	}

}