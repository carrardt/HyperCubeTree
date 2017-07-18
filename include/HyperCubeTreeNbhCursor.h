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

			template<unsigned int D, typename M1, typename M2>
			inline void operator () (
				const HyperCube<HCubeComponentValue,0, M1>& parent,
				HyperCube<HCubeComponentValue, 0, M2>& child, 
				GridDimension<D> grid, 
				GridLocation inCoord, 
				GridLocation outCoord)
			{
				if (parent.value.m_cell.isTreeCell() )
				{
					// FIXME: add sliding information, as in legacy code "amr2ugrid/AmrConnect.h"
					// NOT as in AmrConnect.h, it's buggy :-(
					if ( !m_tree.isLeaf(parent.value.m_cell) )
					{
						child.value.m_cell = m_tree.child(parent.value.m_cell, outCoord);
					}
					else
					{
						child.value.m_cell = parent.value.m_cell; // just a neighbor from a less deep level
					}
				}
				else
				{
					child.value.m_cell = parent.value.m_cell; // this is the case for 'nil' cells, or extra cells added to surround root node.
				}
			}
			const Tree & m_tree;
		};

		// recursion constructor
		inline HyperCubeTreeNbhCursor(Tree& tree, HyperCubeTreeNbhCursor parent, SubdivisionGrid grid, GridLocation childLocation)
		{
			assert(!tree.isLeaf(parent.cell()));
			HyperCubeNeighbor<HCubeComponentValue, D>::dig(grid, parent.m_nbh, m_nbh, childLocation, AttachChildNeighbor(tree) );
		}

		inline Cell cell() const
		{
			return m_nbh.self().m_cell;
		}

		HCube m_nbh;
	};

}
