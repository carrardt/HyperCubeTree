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
	struct HyperCubeTreeNeighborCursor
	{
		using Tree = _Tree;
		static constexpr unsigned int D = Tree::D;
		using Cell = hct::HyperCubeTreeCell;

		// Type of the value to be stored at each neighborhood hypercube's component
		struct HCubeComponentValue
		{
			inline hct::HyperCubeTreeCell cell() const { return m_cell; }
			inline Vec<size_t, D> resolution() const { return m_resolution; }
			inline Vec<size_t, D> position() const { return m_position; }

			hct::HyperCubeTreeCell m_cell; // neighbor cell
			Vec<size_t, D> m_resolution; // resolution in which position is expressed
			Vec<size_t, D> m_position; // poisition of the cell
		};

		using HCube = HyperCube< HCubeComponentValue, D >;
		using SubdivisionGrid = typename Tree::SubdivisionGrid;
		using GridLocation = typename Tree::GridLocation;

		// initialization constructors
		inline HyperCubeTreeNeighborCursor(hct::HyperCubeTreeCell cell = hct::HyperCubeTreeCell())
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
		struct AttachChildNeighborFunctor
		{
			inline AttachChildNeighborFunctor(const Tree & tree) : m_tree(tree) {}

			template<unsigned int D, typename M1, typename M2>
			inline void operator () (
				const HyperCube<HCubeComponentValue,0,M1>& parentNeighbor,
				HyperCube<HCubeComponentValue,0,M2>& childNeighbor, 
				GridDimension<D> subdivisionGrid, 
				GridLocation childLocation, 
				GridLocation neighborChildLocation)
			{
				if ( !m_tree.isTerminal(parentNeighbor.value.m_cell) )
				{
					childNeighbor.value.m_cell = m_tree.child(parentNeighbor.value.m_cell, neighborChildLocation);
					childNeighbor.value.m_resolution = parentNeighbor.value.m_resolution * subdivisionGrid;
					childNeighbor.value.m_position = parentNeighbor.value.m_position * subdivisionGrid + neighborChildLocation;
				}
				else
				{
					childNeighbor.value = parentNeighbor.value;
				}
			}
			const Tree & m_tree;
		};

		// recursion constructor
		inline HyperCubeTreeNeighborCursor(const Tree& tree, const HyperCubeTreeNeighborCursor& parent, SubdivisionGrid grid, GridLocation childLocation)
		{
			assert(!tree.isLeaf(parent.cell()));
			HyperCubeNeighbor<HCubeComponentValue, D>::dig(grid, parent.m_nbh, m_nbh, childLocation, AttachChildNeighborFunctor(tree) );
		}

		inline Cell cell() const
		{
			return m_nbh.self().m_cell;
		}

		inline Vec<size_t, D> position() const
		{
			return m_nbh.self().m_position;
		}

		inline Vec<size_t, D> resolution() const
		{
			return m_nbh.self().m_resolution;
		}

		HCube m_nbh;
	};

}
