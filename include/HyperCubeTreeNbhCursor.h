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
		using HCube = HyperCube< Cell , D >;
		using SubdivisionGrid = typename Tree::SubdivisionGrid;
		using GridLocation = typename Tree::GridLocation;

		// initialization constructors
		inline HyperCubeTreeNbhCursor(hct::HyperCubeTreeCell cell = hct::HyperCubeTreeCell())
		{
			m_nbh.forEachValue([](hct::HyperCubeTreeCell& cell) { cell = hct::HyperCubeTreeCell::nil(); });
			m_nbh.self() = cell;
		}

		// Functor to be applied on parent/child pairs upon recursion
		struct AttachChildNeighbor
		{
			inline AttachChildNeighbor(const Tree & tree) : m_tree(tree) {}

			template<unsigned int D, typename M1, typename M2>
			inline void operator () (const HyperCube<Cell, 0, M1>& parent, HyperCube<Cell, 0, M2>& child, GridDimension<D> grid, GridLocation inCoord, GridLocation outCoord)
			{
				if (parent.value.isTreeCell())
				{
					// FIXME: add sliding information, as in legacy code "amr2ugrid/AmrConnect.h"
					if (!m_tree.isLeaf(parent.value))
					{
						child.value = m_tree.child(parent.value, outCoord);
					}
					else
					{
						child.value = parent.value; // just a neighbor from a less deep level
					}
				}
				else
				{
					child.value = parent.value; // this is the case for 'nil' cells, or extra cells added to surround root node.
				}
			}
			const Tree & m_tree;
		};

		// recursion constructor
		inline HyperCubeTreeNbhCursor(Tree& tree, HyperCubeTreeNbhCursor parent, SubdivisionGrid grid, GridLocation childLocation)
		{
			assert(!tree.isLeaf(parent.cell()));
			HyperCubeNeighbor<Cell, D>::dig(grid, parent.m_nbh, m_nbh, childLocation, AttachChildNeighbor(tree) );
		}

		inline Cell cell() const
		{
			return m_nbh.self();
		}

		HCube m_nbh;
	};

}
