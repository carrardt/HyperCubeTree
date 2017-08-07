#pragma once

#include "ScalarFunction.h"

#include <string>
#include <memory>
#include <vector>
#include <cstdlib>

namespace hct
{

	template<unsigned int D, typename T, typename StreamT>
	ScalarFunctionDelegate<D, T> scalar_function_read(StreamT& input)
	{
		std::string token;
		input >> token;
		if (token == "distance")
		{
			T center[D];
			for (size_t i = 0; i < D; i++) { center[i] = 0.0;  input >> center[i]; }
			auto surf = point_distance_function( Vec<T, D>(center) );
			return scalar_function_delegate(surf);
		}
		else if (token == "constant")
		{
			T c = 0;
			input >> c;
			auto surf = ConstantFunction<D,T>( c );
			return scalar_function_delegate(surf);
		}
		else if (token == "sphere")
		{
			T center[D];
			for (size_t i = 0; i < D; i++) { center[i] = 0.0;  input >> center[i]; }
			T radius = 0;
			input >> radius;
			auto surf = csg_sphere(Vec<T, D>(center), radius);
			return scalar_function_delegate(surf);
		}
		else if (token == "plane")
		{
			T normal[D];
			for (size_t i = 0; i < D; i++) { normal[i] = 0.0;  input >> normal[i]; }
			T offset = 0;
			input >> offset;
			auto surf = plane_function(Vec<T, D + 1>(offset, Vec<T, D>(normal)));
			return scalar_function_delegate(surf);
		}
		else if (token == "-")
		{
			auto a = scalar_function_read<D, T>(input);
			auto surf = negate_function(a);
			return scalar_function_delegate(surf);
		}
		else if (token == "add")
		{
			auto a = scalar_function_read<D, T>(input);
			auto b = scalar_function_read<D, T>(input);
			auto surf = add_function(a,b);
			return scalar_function_delegate(surf);
		}
		else if (token == "csg_union")
		{
			auto a = scalar_function_read<D,T>(input);
			auto b = scalar_function_read<D,T>(input);
			auto surf = csg_union(a, b);
			return scalar_function_delegate(surf);
		}
		else if (token == "csg_intersection")
		{
			auto a = scalar_function_read<D,T>(input);
			auto b = scalar_function_read<D,T>(input);
			auto surf = csg_intersection(a, b);
			return scalar_function_delegate(surf);
		}
		else if (token == "csg_difference")
		{
			auto a = scalar_function_read<D,T>(input);
			auto b = scalar_function_read<D,T>(input);
			auto surf = csg_difference(a, b);
			return scalar_function_delegate(surf);
		}
		/*
		else if( token == "if" )
		{
			...
		}
		*/


		std::abort();
		return scalar_function_delegate(hct::ConstantFunction<D, T>(0));
	}

}