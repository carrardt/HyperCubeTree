#pragma once

#include "HyperCubeTreeCell.h"
#include "HyperCube.h"
#include "HyperCubeNeighbor.h"
#include "HyperCubeTreeNeighborCursor.h"

#include <array>
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
		using OwnershipArray = std::array<bool, NumberOfVertices>;

		// initialization constructors
		inline HyperCubeTreeVertexOwnershipCursor(const Tree& tree, hct::HyperCubeTreeCell cell = hct::HyperCubeTreeCell())
			: SuperClass(cell)
		{
			computeOwnership(tree);
		}

		// recursion constructor
		inline HyperCubeTreeVertexOwnershipCursor(Tree& tree, const HyperCubeTreeVertexOwnershipCursor& parent, SubdivisionGrid grid, GridLocation childLocation)
			: SuperClass(tree,parent,grid, childLocation)
		{
			computeOwnership(tree);
		}

		inline bool ownsVertex(size_t i) const
		{
			return m_ownVertex[i];
		}

	private:

		template<typename _M>
		struct LooseOwnershipFunctor
		{
			inline LooseOwnershipFunctor(OwnershipArray& ownership)	: m_ownVertex(ownership) {}
			template<typename VertBF> inline void operator () (VertBF)
			{
				m_ownVertex[VertBF::BITFIELD] = false;
			}
			OwnershipArray& m_ownVertex;
		};

		template<typename _M>
		struct NeighborComponentVertexFunctor
		{
			using CompBF = typename hct::HyperCube<HCubeComponentValue, 0, _M>::Mask;

			inline NeighborComponentVertexFunctor(HyperCubeTreeVertexOwnershipCursor& cursor, hct::HyperCube<HCubeComponentValue, 0, _M>& neighbor, OwnershipArray& ownership)
				: m_cursor(cursor)
				, m_neighbor(neighbor)
				, m_ownVertex(ownership)
			{}

			/*
			prerequisites :
				neighbor is a tree cell (not nil)
				neighbor is a leaf
				current cell is a leaf
			*/
			template<typename VertBF> inline void operator () (VertBF)
			{
				// Note : this shoud be even more trivial.
				// if we state that (meVertexPos == nbVertexPos).reduce_and() must be true, cell and neighbor have the same level,
				// then we could just determine nbCellPos.less(meCellPos) from CompBF/VertBF

				// VertBF => considered vertex
				// m_cursor => global info about traversed cell of interest
				// m_neighbor => neighbor potentially sharing considered vertex
				size_t meVertex = VertBF::BITFIELD;
				size_t compMask = CompBF::DEF_BITFIELD;
				size_t nbVertex = meVertex ^ compMask;
				hct::Vec<size_t, D> meVertexPos = (m_cursor.position() + hct::bitfield_vec<D>(meVertex)) *  m_neighbor.value.m_resolution;
				hct::Vec<size_t, D> nbVertexPos = (m_neighbor.value.m_position + hct::bitfield_vec<D>(nbVertex)) * m_cursor.resolution();

				assert( (meVertexPos == nbVertexPos).reduce_and() );
				if ((meVertexPos == nbVertexPos).reduce_and()) // if me and neighbor share this vertex and both are leaves, then ...
				{
					hct::Vec<size_t, D> meCellPos = m_cursor.position() *  m_neighbor.value.m_resolution;
					hct::Vec<size_t, D> nbCellPos = m_neighbor.value.m_position * m_cursor.resolution();
					if ( nbCellPos.less(meCellPos) ) // neighbor has priority, i loose ownership
					{
						m_ownVertex[meVertex] = false;
					}
				}
			}

			HyperCubeTreeVertexOwnershipCursor& m_cursor;
			hct::HyperCube<HCubeComponentValue, 0, _M>& m_neighbor;
			OwnershipArray& m_ownVertex;
		};

		struct NeighborComponentFunctor
		{
			inline NeighborComponentFunctor(const Tree& tree, HyperCubeTreeVertexOwnershipCursor& cursor, OwnershipArray& ownership)
				: m_tree(tree)
				, m_cursor(cursor)
				, m_ownVertex(ownership)
			{}

			/*
			prerequistes :
				current cell is a leaf (otherwise all vertices ownership is lost)
			*/
			template<typename _M>
			inline void operator () (hct::HyperCube<HCubeComponentValue, 0, _M>& neighbor)
			{
				using HCubeComp = typename hct::HyperCube<HCubeComponentValue, 0, _M>::Mask;
				if (HCubeComp::N_DEF > 0) // neighbor is not myself
				{
					// if neighbor is not a tree cell (e.g. nil), it cannot still any ownership
					if (neighbor.value.m_cell.isTreeCell())
					{
						// if neighbor has sub-cells, then it sub-cells wil still all shared vertices ownership
						if (!m_tree.isLeaf(neighbor.value.m_cell))
						{
							assert(neighbor.value.m_cell.level() == m_cursor.cell().level());
							HCubeComp::enumerate(LooseOwnershipFunctor<_M>(m_ownVertex));
						}
						else if( neighbor.value.m_cell.level() == m_cursor.cell().level() )
						{
							// neighbor can still ownership only if it has the same level (not coarser)
							HCubeComp::enumerate(NeighborComponentVertexFunctor<_M>(m_cursor, neighbor, m_ownVertex));
						}
					}
				}
			}

			const Tree& m_tree;
			HyperCubeTreeVertexOwnershipCursor& m_cursor;
			OwnershipArray& m_ownVertex;
		};

		void computeOwnership(const Tree& tree)
		{
			if (tree.isLeaf(SuperClass::cell()))
			{
				m_ownVertex.fill(true);
				SuperClass::m_nbh.forEachComponent(NeighborComponentFunctor(tree,*this, m_ownVertex));
			}
			else
			{
				m_ownVertex.fill(false);
			}
		}

		OwnershipArray m_ownVertex;
	};

}
