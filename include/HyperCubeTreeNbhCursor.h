#pragma once

#include "HyperCubeTreeCell.h"
#include "HyperCube.h"
#include "HyperCubeNeighbor.h"
#include <assert.h>

namespace hct
{

	/*
	A tree traversal cursor adding information about position and size of the traversed cell.
	*/
	template<typename _Tree>
	struct HyperCubeTreeNbhCursor
	{
		using Tree = _Tree;
		static constexpr unsigned int D = Tree::D;
		using Cell = hct::HyperCubeTreeCell;

		// Type of the value to be stored at each neighborhood hypercube's component
		struct HCubeComponentValue
		{
			Cell m_cell;			// neighbor cell
			Vec<size_t, D> m_shift;	// shift relative to neighbor cell, if neighbor cell is higher in the tree than cell of interest.
		};

		using HCube = HyperCube< HCubeComponentValue, D >;
		using SubdivisionGrid = typename Tree::SubdivisionGrid;
		using GridLocation = typename Tree::GridLocation;

		// initialization constructors
		inline HyperCubeTreeNbhCursor(hct::HyperCubeTreeCell cell = hct::HyperCubeTreeCell())
		{
			m_nbh.forEachValue([](HCubeComponentValue& comp )
				{ 
					comp.m_cell = hct::HyperCubeTreeCell::nil();
					comp.m_shift = Vec<size_t, D>(0);
				});
			m_nbh.self().m_cell = cell;
		}

		// Functor to be applied on parent/child pairs upon recursion
		struct AttachChildNeighbor
		{
			inline AttachChildNeighbor(const Tree & tree) : m_tree(tree) {}

			template<unsigned int D, typename ParentNeighborMask, typename ChildNeighborMask>
			inline void operator () (
				const HyperCube<HCubeComponentValue,D>& parent,
				HyperCube<HCubeComponentValue,D>& child, 
				GridDimension<D> grid, 
				GridLocation inCoord, 
				GridLocation outCoord,
				ParentNeighborMask pm,
				ChildNeighborMask cm)
			{
				if (parent[pm].m_cell.isTreeCell() )
				{
					// FIXME: add sliding information, as in legacy code "amr2ugrid/AmrConnect.h"
					// NOT as in AmrConnect.h, it's buggy :-(
					if ( !m_tree.isLeaf(parent[pm].m_cell) )
					{
						child[cm].m_cell = m_tree.child(parent[pm].m_cell, outCoord);
					}
					else
					{
						child[cm].m_cell = parent[pm].m_cell; // just a neighbor from a less deep level
					}
				}
				else
				{
					child[cm].m_cell = parent[pm].m_cell; // this is the case for 'nil' cells, or extra cells added to surround root node.
				}
			}
			const Tree & m_tree;
		};

		// recursion constructor
		inline HyperCubeTreeNbhCursor(Tree& tree, HyperCubeTreeNbhCursor parent, SubdivisionGrid grid, GridLocation childLocation)
		{
			assert(!tree.isLeaf(parent.cell()));
			HyperCubeNeighbor2<HCubeComponentValue, D>::dig(grid, parent.m_nbh, m_nbh, childLocation, AttachChildNeighbor(tree) );
		}

		inline Cell cell() const
		{
			return m_nbh.self().m_cell;
		}

		HCube m_nbh;
	};

}