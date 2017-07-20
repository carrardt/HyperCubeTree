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
			Cell m_cell; // neighbor cell
			Vec<size_t, D> m_resolution; // resolution in which position is expressed
			Vec<size_t, D> m_position; // poisition of the cell
		};

		using HCube = HyperCube< HCubeComponentValue, D >;
		using SubdivisionGrid = typename Tree::SubdivisionGrid;
		using GridLocation = typename Tree::GridLocation;

		// initialization constructors
		inline HyperCubeTreeNbhCursor(hct::HyperCubeTreeCell cell = hct::HyperCubeTreeCell())
		{
			// TODO: modify this so that initial 'nil' neighbors are correctly positionned
			m_nbh.forEachValue([](HCubeComponentValue& comp )
				{ 
					comp.m_cell = hct::HyperCubeTreeCell::nil();
					comp.m_resolution = Vec<size_t, D>(1);
					comp.m_position = Vec<size_t, D>(0); // questionable, should be adjusted
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
				GridLocation childLocation, 
				GridLocation neighborChildLocation,
				ParentNeighborMask pm,
				ChildNeighborMask cm)
			{
				if ( !m_tree.isTerminal(parent[pm].m_cell) )
				{
					child[cm].m_cell = m_tree.child(parent[pm].m_cell, neighborChildLocation);
					child[cm].m_resolution = parent[pm].m_resolution * grid;
					child[cm].m_position = parent[pm].m_position * grid + neighborChildLocation;
				}
				else
				{
					child[cm] = parent[pm];
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
