#pragma once

#include "HyperCubeTreeVertexOwnershipCursor.h"
#include "HyperCubeTreeCell.h"
#include "HyperCubeTreeCellPosition.h"
#include "Vec.h"

namespace hct
{

	template<typename _Tree>
	struct HyperCubeTreeDualMesh
	{
		using Tree = _Tree;
		using HCTVertexOwnershipCursor = hct::HyperCubeTreeVertexOwnershipCursor<Tree>;
		using HCubeComponentValue = typename HCTVertexOwnershipCursor::HCubeComponentValue;
		static constexpr unsigned int D = Tree::D;
		static constexpr size_t CellNumberOfVertices = static_cast<size_t>(1) << D;
		using Cell = hct::HyperCubeTreeCell;
		using CellPosition = hct::HyperCubeTreeCellPosition<D>;

		struct DualVertex
		{
			Cell m_cell;		// the cell in the primary mesh
			CellPosition m_coord;  // center of the primary cell
		};

		struct DuallCell
		{
			CellPosition m_center;
			std::array<DualVertex, CellNumberOfVertices> m_vertices;
		};

		template<typename VertBF, typename FuncT>
		struct VertexNeighborCellFunctor
		{
			inline VertexNeighborCellFunctor(const HCTVertexOwnershipCursor& cursor, DuallCell& dual)
				: m_cursor(cursor)
				, m_dual(dual)
			{}

			template<typename T, typename CompBF>
			inline void operator() (const T& neighbor, CompBF)
			{
				using NeighborDualVertBF = typename NeighborDualVertex<CompBF, VertBF>::Vertex;
				constexpr size_t originalVertex = VertBF::BITFIELD;
				constexpr size_t dualVertex = NeighborDualVertBF::BITFIELD;
				m_dual.m_vertices[dualVertex].m_cell = neighbor.cell();
				m_dual.m_vertices[dualVertex].m_coord = neighbor.m_position.addHalfUnit(); // middle of the cell
			}

			const HCTVertexOwnershipCursor& m_cursor;
			DuallCell& m_dual;
		};

		template<typename FuncT>
		struct VertexFunctor
		{
			inline VertexFunctor(const HCTVertexOwnershipCursor& cursor, FuncT f)
				: m_cursor(cursor)
				, m_f(f)
			{}

			template<typename T, typename VertBF>
			inline void operator() (const T& value, VertBF)
			{
				constexpr size_t i = VertBF::BITFIELD;
				if (m_cursor.ownsVertex(i))
				{
					DuallCell dual;
					dual.m_center = m_cursor.position() + hct::bitfield_vec<D>(i);
					m_cursor.m_nbh.forEachComponentSharingVertex(VertBF(), VertexNeighborCellFunctor<VertBF,FuncT>(m_cursor, dual ));
					m_f(dual);
				}
			}

			const HCTVertexOwnershipCursor& m_cursor;
			FuncT m_f;
		};

		template<typename FuncT>
		static inline
		void parseDualCells(const Tree& tree, FuncT f)
		{
			tree.parseLeaves([f](const HCTVertexOwnershipCursor& cursor)
			{
				cursor.m_nbh.forEachVertexComponent(VertexFunctor<FuncT>(cursor,f));
			}
			, HCTVertexOwnershipCursor(tree));
		}

	};

}