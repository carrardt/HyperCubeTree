#include "HyperCubeTree.h"
#include "SimpleSubdivisionScheme.h"
#include "GridDimension.h"
#include "TreeLevelStorage.h"

#include <iostream>
#include <algorithm>

template<typename T, unsigned int _D>
struct PointDistanceFunctor
{
	static constexpr unsigned int D = _D;
	using VecT = hct::Vec<T, D>;

	inline PointDistanceFunctor(std::initializer_list<double> l, double scale=1.0, double offset=0.0)
		: m_center(l.begin())
		, m_scale(scale)
		, m_offset(offset) {}

	inline T operator () (VecT p)
	{
		VecT v = p - m_center;
		return v.dot(v) * m_scale + m_offset;
	}
	VecT m_center;
	double m_scale = 1.0;
	double m_offset = 0.0;
};

template<typename T, unsigned int _D, typename FuncOpT, typename Func1T, typename Func2T>
struct ScalarBinaryPointFunction
{
	static constexpr unsigned int D = _D;
	using VecT = hct::Vec<T, D>;

	inline ScalarBinaryPointFunction(FuncOpT opf, Func1T f1, Func2T f2)
		: m_op(opf)
		, m_f1(f1)
		, m_f2(f2)
	{}

	inline T operator () (VecT p)
	{
		return m_op(m_f1(p), m_f2(p));
	}

	FuncOpT m_op;
	Func1T m_f1;
	Func2T m_f2;
};



int main()
{
	unsigned int div_4x4x20[3] = { 4, 4, 20 };
	unsigned int div_3x3x3[3] = { 3, 3, 3 };

	hct::SimpleSubdivisionScheme<3> subdivisions;
	subdivisions.addLevelSubdivision(hct::Vec<unsigned int, 3>(div_4x4x20));
	subdivisions.addLevelSubdivision(hct::Vec<unsigned int, 3>(div_3x3x3));
	subdivisions.addLevelSubdivision(hct::Vec<unsigned int, 3>(div_3x3x3));
	subdivisions.addLevelSubdivision(hct::Vec<unsigned int, 3>(div_3x3x3));
	subdivisions.addLevelSubdivision(hct::Vec<unsigned int, 3>(div_3x3x3));

	hct::HyperCubeTree< 3, hct::SimpleSubdivisionScheme<3> > tree(subdivisions);

	hct::TreeLevelArray<double> cellValues;
	tree.addArray(&cellValues);

	tree.toStream(std::cout);

	// rafiner la racine
	std::cout << "subdivide root\n";
	tree.refine(tree.rootCell());

	tree.toStream(std::cout);

	// rafiner entierement le premier niveau après la racine
	std::cout << "subdivide 2nd level cells\n";
	size_t nbRootChildren = subdivisions.getLevelSubdivision(0).gridSize();
	for (size_t i = 0; i < nbRootChildren; i++)
	{
		tree.refine(tree.child(tree.rootCell(),i));
	}
	tree.toStream(std::cout);

	std::cout << "initialize cellValues" << std::endl;
	tree.preorderParseCells([&cellValues](HyperCubeTreeCell cell) { cellValues[cell] = cell.m_level*100000000.0 + cell.m_index; });

	double maxval = 0.0;
	std::cout << "read cellValues" << std::endl;
	tree.preorderParseCells([&maxval,&cellValues](HyperCubeTreeCell cell) { maxval = std::max(maxval,cellValues[cell]); });
	std::cout << "max value = " << maxval << std::endl;

	return 0;
}
