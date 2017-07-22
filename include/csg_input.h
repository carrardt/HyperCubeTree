#pragma once

#include "csg.h"
#include <string>
#include <memory>
#include <vector>
#include <cstdlib>

namespace hct
{
	template<unsigned int D, typename StreamT>
	CSGSurfaceDelegate<D> csg_input(StreamT& input)
	{
		std::vector< CSGSurfaceDelegate<D> > stack;
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
				stack.push_back( CSGSurfaceDelegate<D>( csg_sphere_new(Vec<double,D>(center),radius) ) );
			}
			else if (token == "plane")
			{
				double normal[D];
				for (size_t i = 0; i < D; i++) { normal[i] = 0.0;  input >> normal[i]; }
				double offset = 0;
				input >> offset;
				stack.push_back(CSGSurfaceDelegate<D>(csg_plane_new(Vec<double, D>(normal), offset) ) );
			}
			else if (token == "union")
			{
				auto a = stack.back(); stack.pop_back();
				auto b = stack.back(); stack.pop_back();
				stack.push_back( CSGSurfaceDelegate<D>( csg_union_new(a,b) ) );
			}
			else if (token == "intersection")
			{
				auto a = stack.back(); stack.pop_back();
				auto b = stack.back(); stack.pop_back();
				stack.push_back( CSGSurfaceDelegate<D>( csg_intersection_new(a,b) ) );
			}
			else if (token == "difference")
			{
				auto a = stack.back(); stack.pop_back();
				auto b = stack.back(); stack.pop_back();
				stack.push_back(CSGSurfaceDelegate<D>(csg_intersection_new(a, csg_negate(b) ) ) );
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