#pragma once

#include <iostream>
#include <array>
#include <string>

// use out.fill('x') to use x as a separator
template<typename T, size_t N>
inline std::ostream& operator << (std::ostream& out, const std::array<T, N>& cv)
{
	for (size_t i = 0; i < N; i++)
	{
		if (i > 0)
		{
			out.width(1);
			out << "";
		}
		out << cv[i];
	}
	return out;
}
