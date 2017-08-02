#include "HyperCubeTreeCellPosition.h"

#include <iostream>
#include <assert.h>

using CellPostion = hct::HyperCubeTreeCellPosition<3>;
using Vec3i = hct::Vec<size_t,3>;
using hct::Vec3d;

template <typename T, unsigned int D>
inline std::ostream & operator << (std::ostream& out, const hct::Vec<T, D>& v)
{
	v.toStream(out);
	return out;
}

inline std::ostream & operator << (std::ostream& out, const CellPostion& v)
{
	v.toStream(out);
	return out;
}

void testCellPositionVariant(CellPostion p)
{
	std::cout << p << " = " << p.normalize() << " : boundary=" << p.boundary()
		<< ", eq0="<<(p.m_position==Vec3i(0)) << '/'<<(p.m_position == Vec3i(0)).reduce_or()
		<<", eqres="<< (p.m_position ==p.m_resolution) << '/'<<(p.m_position == p.m_resolution).reduce_or() << std::endl;
	assert((p.m_position >= Vec3i(0)).reduce_and());
	assert((p.m_position <= p.m_resolution).reduce_and());
}

void testCellPosition(CellPostion p)
{
	testCellPositionVariant(p);
	if (!p.boundary())
	{
		testCellPositionVariant(p.addHalfUnit());
	}
}

int main()
{
	CellPostion p;
	testCellPosition(p);
	for (size_t i = 0; i < 8; i++)
	{
		testCellPosition( p + hct::bitfield_vec<3>(i) );
	}
	p = p.refine(Vec3i({3,3,3}) );
	testCellPosition(p);
	for (size_t i = 0; i < 8; i++)
	{
		testCellPosition(p + hct::bitfield_vec<3>(i));
	}
	p = p + Vec3i({ 2,2,2 });
	testCellPosition(p);
	for (size_t i = 0; i < 8; i++)
	{
		testCellPosition(p + hct::bitfield_vec<3>(i));
	}
	return 0;
}
