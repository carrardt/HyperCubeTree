#pragma once

#include <cstddef>

struct HyperCubeTreeCell
{
	inline HyperCubeTreeCell() : m_level(0), m_index(0) {}
	inline HyperCubeTreeCell(size_t l, size_t i) : m_level(l), m_index(i) {}

	inline size_t level() const { return m_level; }
	inline size_t index() const { return m_index; }

	size_t m_level;
	size_t m_index;
};
