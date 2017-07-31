#pragma once

#include "HyperCubeTreeCell.h"
#include "HyperCube.h"
#include "HyperCubeNeighbor.h"
#include "HyperCubeTreeNeighborCursor.h"
#include "HyperCubeTreeCellPosition.h"

#include <array>
#include <assert.h>

namespace hct
{
	/*
		Important statements :
			1. Owner (a cell) of a vertex is a leaf of the tree
			2. Ownership changes if tree topologie changes (refinment, corsening, etc.)
			3. A vertex is owned by exactly one (leaf) cell
			4. Let C_i be the cells sharing a vertex V, then MIN_i( level(C_i) ) = level( owner(V) )
	*/

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
		using CellPosition = hct::HyperCubeTreeCellPosition<D>;
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
		inline HyperCubeTreeVertexOwnershipCursor(const Tree& tree, const HyperCubeTreeVertexOwnershipCursor& parent, SubdivisionGrid grid, GridLocation childLocation)
			: SuperClass(tree,parent,grid, childLocation)
		{
			computeOwnership(tree);
		}

		inline bool ownsVertex(size_t i) const
		{
			return m_ownVertex[i];
		}

	private:

		struct LooseOwnershipFunctor
		{
			inline LooseOwnershipFunctor(OwnershipArray& ownership)	: m_ownVertex(ownership) {}
			template<typename VertBF> inline void operator () (VertBF)
			{
				m_ownVertex[VertBF::BITFIELD] = false;
			}
			OwnershipArray& m_ownVertex;
		};

		template<typename CompBF>
		struct NeighborComponentVertexFunctor
		{
			inline NeighborComponentVertexFunctor(HyperCubeTreeVertexOwnershipCursor& cursor, HCubeComponentValue& neighbor, OwnershipArray& ownership)
				: m_cursor(cursor)
				, m_neighbor(neighbor)
				, m_ownVertex(ownership)
			{}

			/*
			prerequisites :
				neighbor is a tree cell (not nil)
				neighbor is a leaf
				current cell is a leaf
				neighbor and current cell have are at the same level
			As a consequence :
				neighbor vertex is shared with (is the same as) current cell's corresponding vertex (nbVertex==meVertex)
			*/
			template<typename VertBF> inline void operator () (VertBF)
			{
				// VertBF => considered vertex
				// m_cursor => global info about traversed cell of interest
				// m_neighbor => neighbor sharing considered vertex
				constexpr size_t meVertex = VertBF::BITFIELD;
				CellPosition meCellPos = m_cursor.position();
				CellPosition nbCellPos = m_neighbor.m_position;
#				ifndef NDEBUG
				constexpr size_t compMask = CompBF::DEF_BITFIELD;
				constexpr size_t nbVertex = meVertex ^ compMask;
				CellPosition meVertexPos = meCellPos + hct::bitfield_vec<D>(meVertex);
				CellPosition nbVertexPos = nbCellPos  + hct::bitfield_vec<D>(nbVertex);
				assert( meVertexPos == nbVertexPos );
#				endif
				if ( nbCellPos < meCellPos ) // neighbor has priority, i loose ownership
				{
					m_ownVertex[meVertex] = false;
				}
			}

			HyperCubeTreeVertexOwnershipCursor& m_cursor;
			HCubeComponentValue& m_neighbor;
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
				current cell is a leaf (otherwise all vertices ownership are lost)
			*/
			template<typename HCubeComp>
			inline void operator () (HCubeComponentValue& neighbor, HCubeComp)
			{
				if (HCubeComp::N_DEF > 0) // neighbor is not myself
				{
					// if neighbor is not a tree cell (e.g. nil), it cannot still any ownership
					if (neighbor.m_cell.isTreeCell())
					{
						// if neighbor has sub-cells, then it sub-cells will still all shared vertices ownership
						if (!m_tree.isLeaf(neighbor.m_cell))
						{
							assert(neighbor.m_cell.level() == m_cursor.cell().level());
							HCubeComp::enumerate(LooseOwnershipFunctor(m_ownVertex));
						}
						else if( neighbor.m_cell.level() == m_cursor.cell().level() )
						{
							// neighbor can still ownership only if it has the same level (not coarser)
							HCubeComp::enumerate(NeighborComponentVertexFunctor<HCubeComp>(m_cursor, neighbor, m_ownVertex));
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
