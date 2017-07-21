#pragma once

#include "HyperCubeTreeCell.h"
#include "HyperCube.h"
#include "HyperCubeNeighbor.h"
#include "HyperCubeTreeNeighborCursor.h"

#include <assert.h>

namespace hct
{

	/*
	Essentially identical to HyperCubeTreeNeighborCursor, except it keeps track of vertex ownership
	*/
	template<typename _Tree>
	struct HyperCubeTreeVertexOwnershipCursor : public HyperCubeTreeNeighborCursor<_Tree>
	{
		using SuperClass = HyperCubeTreeNeighborCursor<_Tree>;
		using Tree = _Tree;
		static constexpr unsigned int D = Tree::D;
		static constexpr size_t NumberOfVertices = static_cast<size_t>(1) << D;
		using Cell = hct::HyperCubeTreeCell;
		using SubdivisionGrid = typename Tree::SubdivisionGrid;
		using GridLocation = typename Tree::GridLocation;
		using HCubeComponentValue = typename SuperClass::HCubeComponentValue;

		// initialization constructors
		inline HyperCubeTreeVertexOwnershipCursor(hct::HyperCubeTreeCell cell = hct::HyperCubeTreeCell())
			: SuperClass(cell)
			, m_nOwnVertices(0)
		{
			computeOwnership();
		}

		// recursion constructor
		inline HyperCubeTreeVertexOwnershipCursor(Tree& tree, const HyperCubeTreeVertexOwnershipCursor& parent, SubdivisionGrid grid, GridLocation childLocation)
			: SuperClass(tree,parent,grid, childLocation)
			, m_nOwnVertices(0)
		{
			computeOwnership();
		}

		inline size_t getNumberOfOwnedVertices() const
		{
			return m_nOwnVertices;
		}

		inline size_t getOwnVertex(size_t i) const
		{
			return m_ownVertex[i];
		}

	private:

		template<typename _M>
		struct NeighborComponentVertexFunctor
		{
			using CompBF = typename hct::HyperCube<HCubeComponentValue, 0, _M>::Mask;

			inline NeighborComponentVertexFunctor(HyperCubeTreeVertexOwnershipCursor& cursor, hct::HyperCube<HCubeComponentValue, 0, _M>& neighbor, Cell* owner)
				: m_cursor(cursor)
				, m_neighbor(neighbor)
				, m_owner(owner)
			{}

			template<typename VertBF> inline void operator () (VertBF)
			{
				// VertBF => considered vertex
				// m_cursor => global info about traversed cell of interest
				// m_neighbor => neighbor potentially sharing considered vertex
				hct::HyperCubeTreeCell meCell = m_cursor.cell();
				hct::HyperCubeTreeCell nbCell = m_neighbor.value.m_cell;
				if (nbCell.isTreeCell() && nbCell.level() == meCell.level())
				{
					size_t meVertex = VertBF::BITFIELD;
					size_t compMask = CompBF::DEF_BITFIELD;
					size_t nbVertex = meVertex ^ compMask;
					hct::Vec<size_t, D> meVertexPos = (m_cursor.position() + hct::bitfield_vec<D>(meVertex)) *  m_neighbor.value.m_resolution;
					hct::Vec<size_t, D> nbVertexPos = (m_neighbor.value.m_position + hct::bitfield_vec<D>(nbVertex)) * m_cursor.resolution();

					if ((meVertexPos == nbVertexPos).reduce_and())
					{
						hct::Vec<size_t, D> meCellPos = m_cursor.position() *  m_neighbor.value.m_resolution;
						hct::Vec<size_t, D> nbCellPos = m_neighbor.value.m_position * m_cursor.resolution();
						if (!tree.isLeaf(nbCell) || nbCellPos.less(meCellPos))
						{
							m_owner[meVertex] = HyperCubeTreeCell::nil();
						}
					}
				}
			}

			HyperCubeTreeVertexOwnershipCursor& m_cursor;
			hct::HyperCube<HCubeComponentValue, 0, _M>& m_neighbor;
			Cell* m_owner;
		};

		struct NeighborComponentFunctor
		{
			inline NeighborComponentFunctor(HyperCubeTreeVertexOwnershipCursor& cursor, Cell* owner)
				: m_cursor(cursor)
				, m_owner(owner)
			{}

			template<typename _M>
			inline void operator () (hct::HyperCube<HCubeComponentValue, 0, _M>& neighbor)
			{
				using HCubeComp = typename hct::HyperCube<HCubeComponentValue, 0, _M>::Mask;
				if (HCubeComp::N_DEF > 0)
				{
					HCubeComp::enumerate(NeighborComponentVertexFunctor<_M>(m_cursor, neighbor, m_owner));
				}
			}

			HyperCubeTreeVertexOwnershipCursor& m_cursor;
			Cell* m_owner;
		};

		void computeOwnership()
		{
			m_nOwnVertices = 0;
			if (tree.isLeaf(SuperClass::cell()))
			{
				for (size_t i = 0; i < NumberOfVertices; i++)
				{
					m_ownVertex[i] = i;
				}
				SuperClass::m_nbh.forEachComponent(NeighborComponentFunctor(*this, owner));
				for (size_t i = 0; i < NumberOfVertices; i++)
				{
					if (m_ownVertex[i] != -1)
					{
						m_ownVertex[m_nOwnVertices] = m_ownVertex[i];
						++m_nOwnVertices;
					}
				}
			}
		}

		size_t m_nOwnVertices;
		int64_t m_ownVertex[NumberOfVertices];
	};

}
