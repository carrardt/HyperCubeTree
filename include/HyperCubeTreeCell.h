#pragma once

#include <cstdint>

struct HyperCubeTreeCell
{
	inline HyperCubeTreeCell() : m_level(0), m_index(0) {}
	size_t m_level;
	size_t m_index;
};
