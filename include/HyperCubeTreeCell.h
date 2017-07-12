#pragma once

#include <cstddef>

struct HyperCubeTreeCell
{
	inline HyperCubeTreeCell() : m_level(0), m_index(0) {}
	inline HyperCubeTreeCell(size_t l, size_t i) : m_level(l), m_index(i) {}
	size_t m_level;
	size_t m_index;
};
