#include "HyperCubeTree.h"
#include "SimpleSubdivisionScheme.h"
#include "GridDimension.h"
#include "csg.h"
#include "HyperCubeTreeLocatedCursor.h"
#include "HyperCubeTreeNeighborCursor.h"

#include <iostream> 

using hct::Vec3d;
std::ostream& operator << (std::ostream& out, Vec3d p) { return p.toStream(out); }
std::ostream& operator << (std::ostream& out, hct::HyperCubeTreeCell c) { return c.toStream(out); }

template<unsigned int D>
struct CellVertices
{
	hct::Vec<double, D> m_vertices[1 << D];
};

using SubdivisionScheme = hct::SimpleSubdivisionScheme<3>;
using Tree = hct::HyperCubeTree<3, SubdivisionScheme>;
using TreeCursor = hct::HyperCubeTreeLocatedCursor<Tree>;
using NbhTreeCursor = hct::HyperCubeTreeNeighborCursor<Tree>;


// ==========================================================================
// ============================ 1st testing method ==========================
// ==========================================================================

static void testTreeNeighborhood(Tree& tree)
{
	const SubdivisionScheme& subdivisions = tree.getSubdivisionScheme();
	Vec3d domain(1.0);
	for (size_t i = 0; i < subdivisions.getNumberOfLevelSubdivisions(); i++)
	{
		domain *= subdivisions.getLevelSubdivision(i);
	}
	std::cout << "Domain bounds = " << domain << std::endl;
	TreeCursor cursor(domain);

	// compute vertex coords
	using CellVerticesT = CellVertices<3>;
	hct::TreeLevelArray< CellVerticesT > cellVertices;
	tree.addArray(&cellVertices);
	tree.preorderParseCells([&tree, &cellVertices](TreeCursor cursor)
	{
		hct::HyperCubeTreeCell cell = cursor.cell();
		constexpr size_t nVertices = 1 << TreeCursor::D;
		for (size_t i = 0; i < nVertices; i++)
		{
			auto vertex = hct::bitfield_vec<TreeCursor::D>(i);
			Vec3d offset = vertex * cursor.m_size;
			Vec3d p = cursor.m_origin + offset;
			//std::cout <<"i="<<i<<", origin=" << cursor.m_origin << ", size=" << cursor.m_size << ", vertex=" << vertex << ", offset=" << offset << ", p=" << p << std::endl;
			cellVertices[cell].m_vertices[i] = p;
		}
	}
	, cursor);

	tree.toStream(std::cout);

	std::cout << "Corner neighbors connection test...\n";
	size_t nbConnectedVertices = 0;
	double maxVertexDist2 = 0.0;
	tree.preorderParseCells([&cellVertices, &nbConnectedVertices,&maxVertexDist2](NbhTreeCursor nbhCursor)
	{
		using HCubeComponentValue = typename NbhTreeCursor::HCubeComponentValue;
		hct::HyperCubeTreeCell me = nbhCursor.cell();
		nbhCursor.m_nbh.forEachVertex([me, &cellVertices, &nbConnectedVertices, &maxVertexDist2](size_t i, const HCubeComponentValue& neighbor)
		{
			if (neighbor.m_cell.isTreeCell() && neighbor.m_cell.level() == me.level())
			{
				size_t j = (i^7);
				Vec3d p = cellVertices[me].m_vertices[i];
				Vec3d n = cellVertices[neighbor.m_cell].m_vertices[j];
				double d = (n - p).dot(n - p);
				maxVertexDist2 = std::max( d, maxVertexDist2 );
				if ( d > 1.e-19 )
				{
					std::cout << "me=" << me << ", i=" << i << ", p=" << p << ", neighbor=" << neighbor.m_cell << ", n=" << n <<", d="<<d <<std::endl;
					abort();
				}
				++nbConnectedVertices;
			}
		});
	}
	, NbhTreeCursor());
	std::cout << nbConnectedVertices << " connected vertices, max dist2 = "<< maxVertexDist2<<"\n";
}


// ==========================================================================
// ============================ 2nd testing method ==========================
// ==========================================================================

template<unsigned int _D> struct Neighbor2ComponentFunctor;

template<typename _M>
struct CompEnumFunc
{
	using HCubeComponentValue = typename NbhTreeCursor::HCubeComponentValue;
	using CompBF = typename hct::HyperCube<HCubeComponentValue, 0, _M>::Mask;
	static constexpr unsigned int D = CompBF::N_BITS;

	inline CompEnumFunc(Neighbor2ComponentFunctor<D>& comp, hct::HyperCube<HCubeComponentValue, 0, _M>& neighbor)
		: m_comp(comp)
		, m_neighbor(neighbor) {}

	template<typename VertBF> inline void operator () (VertBF)
	{
		assert(VertBF::N_FREE == 0);
		const HCubeComponentValue& me = m_comp.m_nbh.self();
		hct::HyperCubeTreeCell meCell = me.m_cell;
		hct::HyperCubeTreeCell nbCell = m_neighbor.value.m_cell;
		assert(nbCell.isTreeCell());

		size_t meVertex = VertBF::BITFIELD;
		size_t compMask = CompBF::DEF_BITFIELD;
		size_t nbVertex = meVertex ^ compMask;

		hct::Vec<size_t, D> meVertexPos = ( me.m_position + hct::bitfield_vec<D>(meVertex) ) *  m_neighbor.value.m_resolution ;
		hct::Vec<size_t, D> nbVertexPos = ( m_neighbor.value.m_position + hct::bitfield_vec<D>(nbVertex) ) * me.m_resolution ;

		if ((meVertexPos == nbVertexPos).reduce_and())
		{
			Vec3d p = m_comp.m_cellVertices[meCell].m_vertices[meVertex];
			Vec3d n = m_comp.m_cellVertices[nbCell].m_vertices[nbVertex];
			double d = (n - p).abs().reduce_max(); // TODO: replace this with L-inf norm (max component wise absolute difference)
			m_comp.m_maxVertexDist2 = std::max(d, m_comp.m_maxVertexDist2);
			if (d > 1.e-16)
			{
				std::cout << "comp="; CompBF::toStream(std::cout);
				std::cout << "meC=" << meCell << ", meV=" << meVertex << ", p=" << p << ", nbC=" << nbCell << ", nbV=" << nbVertex << ", n=" << n << ", d=" << d << std::endl;
				abort();
			}
			++m_comp.m_nbConnectedVertices;
		}
	}

	Neighbor2ComponentFunctor<D>& m_comp;
	hct::HyperCube<HCubeComponentValue, 0, _M>& m_neighbor;
};

template<unsigned int _D>
struct Neighbor2ComponentFunctor
{
	static constexpr unsigned int D = _D;
	using HCubeComponentValue = typename NbhTreeCursor::HCubeComponentValue;
	using CellVertexArray = hct::TreeLevelArray< CellVertices<3> >;
	using HCubeNeighborhood = hct::HyperCube< HCubeComponentValue, D >;

	inline Neighbor2ComponentFunctor(const HCubeNeighborhood& nbh, CellVertexArray& cellVertices, size_t& nbConnectedVertices, double& maxVertexDist2)
		: m_nbh(nbh)
		, m_cellVertices(cellVertices)
		, m_nbConnectedVertices(nbConnectedVertices)
		, m_maxVertexDist2(maxVertexDist2) {}

	template<typename _M>
	inline void operator () ( hct::HyperCube<HCubeComponentValue, 0, _M>& neighbor)
	{
		using HCubeComp = typename hct::HyperCube<HCubeComponentValue, 0, _M>::Mask;
		if ( neighbor.value.m_cell.isTreeCell() )
		{
			HCubeComp::enumerate( CompEnumFunc<_M>(*this, neighbor) );
		}
	}

	const HCubeNeighborhood& m_nbh;
	CellVertexArray& m_cellVertices;
	size_t& m_nbConnectedVertices;
	double& m_maxVertexDist2;
};

static void testTreeNeighborhood2(Tree& tree)
{
	const SubdivisionScheme& subdivisions = tree.getSubdivisionScheme();
	Vec3d domain(1.0);
	for (size_t i = 0; i < subdivisions.getNumberOfLevelSubdivisions(); i++)
	{
		domain *= subdivisions.getLevelSubdivision(i);
	}
	std::cout << "Domain bounds = " << domain << std::endl;
	TreeCursor cursor(domain);

	// compute vertex coords
	using CellVerticesT = CellVertices<3>;
	hct::TreeLevelArray< CellVerticesT > cellVertices;
	tree.addArray(&cellVertices);
	tree.preorderParseCells([&tree, &cellVertices](TreeCursor cursor)
	{
		hct::HyperCubeTreeCell cell = cursor.cell();
		constexpr size_t nVertices = 1 << TreeCursor::D;
		for (size_t i = 0; i < nVertices; i++)
		{
			auto vertex = hct::bitfield_vec<TreeCursor::D>(i);
			Vec3d offset = vertex * cursor.m_size;
			Vec3d p = cursor.m_origin + offset;
			//std::cout <<"i="<<i<<", origin=" << cursor.m_origin << ", size=" << cursor.m_size << ", vertex=" << vertex << ", offset=" << offset << ", p=" << p << std::endl;
			cellVertices[cell].m_vertices[i] = p;
		}
	}
	, cursor);

	tree.toStream(std::cout);

	std::cout << "All neighbor components connection test...\n";
	size_t nbConnectedVertices = 0;
	double maxVertexDist2 = 0.0;
	tree.preorderParseCells([&cellVertices, &nbConnectedVertices, &maxVertexDist2](NbhTreeCursor nbhCursor)
	{
		using HCubeComponentValue = typename NbhTreeCursor::HCubeComponentValue;
		hct::HyperCubeTreeCell me = nbhCursor.cell();
		Neighbor2ComponentFunctor<TreeCursor::D> func(nbhCursor.m_nbh, cellVertices, nbConnectedVertices, maxVertexDist2);
		nbhCursor.m_nbh.forEachComponent(func);
	}
	, NbhTreeCursor());
	std::cout << nbConnectedVertices << " connected vertices, max dist2 = " << maxVertexDist2 << "\n";
}



// ==========================================================================
// ============================ test main ===================================
// ==========================================================================

int main()
{
	{
		std::cout << "\ntest 1a :\n";
		SubdivisionScheme subdivisions;
		subdivisions.addLevelSubdivision({ 2,2,2 });
		Tree tree(subdivisions);
		tree.refine(tree.rootCell());
		testTreeNeighborhood(tree);

		std::cout << "\ntest 1b :\n";
		testTreeNeighborhood2(tree);
	}

	{
		std::cout << "\ntest 2a :\n";
		SubdivisionScheme subdivisions;
		subdivisions.addLevelSubdivision({ 3,3,3 });
		Tree tree(subdivisions);
		tree.refine(tree.rootCell());
		testTreeNeighborhood(tree);

		std::cout << "\ntest 2b :\n";
		testTreeNeighborhood2(tree);
	}

	{
		std::cout << "\ntest 3a :\n";
		SubdivisionScheme subdivisions;
		subdivisions.addLevelSubdivision({ 3,3,3 });
		subdivisions.addLevelSubdivision({ 3,3,3 });
		Tree tree(subdivisions);
		tree.refine(tree.rootCell());
		size_t nbRootChildren = subdivisions.getLevelSubdivision(0).gridSize();
		for (size_t i = 0; i < nbRootChildren; i++)
		{
			tree.refine(tree.child(tree.rootCell(), i));
		}
		testTreeNeighborhood(tree);

		std::cout << "\ntest 3b :\n";
		testTreeNeighborhood2(tree);
	}

	{
		std::cout << "\ntest 4a :\n";
		SubdivisionScheme subdivisions;
		subdivisions.addLevelSubdivision({ 4,4,20 });
		subdivisions.addLevelSubdivision({ 3,3,3 });
		Tree tree(subdivisions);
		tree.refine(tree.rootCell());
		size_t nbRootChildren = subdivisions.getLevelSubdivision(0).gridSize();
		for (size_t i = 0; i < nbRootChildren; i++)
		{
			tree.refine(tree.child(tree.rootCell(), i));
		}
		testTreeNeighborhood(tree);

		std::cout << "\ntest 4b :\n";
		testTreeNeighborhood2(tree);
	}

	{
		std::cout << "\ntest 5a :\n";
		SubdivisionScheme subdivisions;
		subdivisions.addLevelSubdivision({ 4,4,20 });
		subdivisions.addLevelSubdivision({ 3,3,3 });
		subdivisions.addLevelSubdivision({ 3,3,3 });
		subdivisions.addLevelSubdivision({ 3,3,3 });
		subdivisions.addLevelSubdivision({ 3,3,3 });
		Tree tree(subdivisions);
		tree.refine(tree.rootCell());
		size_t nbRootChildren = subdivisions.getLevelSubdivision(0).gridSize();
		for (size_t i = 0; i < nbRootChildren; i++)
		{
			tree.refine(tree.child(tree.rootCell(), i));
		}

		Vec3d domainUnitSize(1.0);
		TreeCursor cursor(domainUnitSize);

		auto sphereA = hct::csg_sphere(Vec3d({ 0.0,0.0,0.0 }), 1.0);
		auto sphereB = hct::csg_sphere(Vec3d({ 0.5,0.5,0.5 }), 0.5);
		auto shape = hct::csg_difference(sphereA, sphereB);

		// refine tree given an implicit surface
		tree.preorderParseCells([shape, &tree](TreeCursor cursor)
		{
			hct::HyperCubeTreeCell cell = cursor.cell();
			if (tree.isRefinable(cell))
			{
				// refine along the implicit surface shape(x)=0
				constexpr size_t nVertices = 1 << TreeCursor::D;
				bool allInside = true;
				bool allOutside = true;
				for (size_t i = 0; i < nVertices; i++)
				{
					auto vertex = hct::bitfield_vec<TreeCursor::D>(i);
					Vec3d p = cursor.m_origin + vertex * cursor.m_size;
					if (shape(p) > 0.0) { allInside = false; }
					else { allOutside = false; }
				}
				if (!allInside && !allOutside)
				{
					tree.refine(cell);
				}
			}
		}
		, cursor);

		testTreeNeighborhood(tree);

		std::cout << "\ntest 5b :\n";
		testTreeNeighborhood2(tree);
	}

	return 0;
}
