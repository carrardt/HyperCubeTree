#pragma once

#include "HyperCubeTreeVertexOwnershipCursor.h"
#include "HyperCubeTreeCell.h"

#include <limits>	// numeric_limits
#include <cstddef>	// size_t

namespace hct
{
	template<typename _Tree>
	struct CellVertexConnectivity
	{
		using Tree = _Tree;
		using DefaultTreeCursor = typename Tree::DefaultTreeCursor;
		using HCTVertexOwnershipCursor = hct::HyperCubeTreeVertexOwnershipCursor<Tree>;
		using HCubeComponentValue = typename HCTVertexOwnershipCursor::HCubeComponentValue;
		static constexpr unsigned int D = Tree::D;
		static constexpr size_t CellNumberOfVertices = static_cast<size_t>(1) << D;
		using Cell = hct::HyperCubeTreeCell;
		static constexpr size_t NotAVertexId = std::numeric_limits<size_t>::max();
		using CellVertexIds = std::array<size_t, CellNumberOfVertices>;
		using VertexIdArray = TreeLevelArray<CellVertexIds>;

		template<typename VertBF>
		struct Pass1VertexNeighborCellFunctor
		{
			inline Pass1VertexNeighborCellFunctor(VertexIdArray& vertexIdArray, size_t vertexId)
				: m_vertexIdArray(vertexIdArray)
				, m_vertexId(vertexId)
			{}

			template<typename T, typename CompBF>
			inline void operator() (const T& neighbor, CompBF)
			{
				using NeighborVertBF = typename NeighborVertex<CompBF, VertBF>::Vertex ;
				constexpr size_t v = NeighborVertBF::BITFIELD;
				assert(m_vertexIdArray[neighbor.cell()][v] == NotAVertexId || m_vertexIdArray[neighbor.cell()][v] == m_vertexId);
				m_vertexIdArray[neighbor.cell()][v] = m_vertexId;
			}

			VertexIdArray& m_vertexIdArray;
			size_t m_vertexId;
		};

		struct Pass1VertexFunctor
		{
			inline Pass1VertexFunctor(const HCTVertexOwnershipCursor& cursor, VertexIdArray& vertexIdArray, size_t& nVertices)
				: m_cursor(cursor)
				, m_vertexIdArray(vertexIdArray)
				, m_nVertices(nVertices)
			{}

			template<typename T, typename VertBF> 
			inline void operator() (const T& value, VertBF)
			{
				constexpr size_t i = VertBF::BITFIELD;
				if (m_cursor.ownsVertex(i))
				{
					assert(m_vertexIdArray[m_cursor.cell()][i] == NotAVertexId);
					m_vertexIdArray[m_cursor.cell()][i] = m_nVertices;
					m_cursor.m_nbh.forEachComponentSharingVertex(VertBF(), Pass1VertexNeighborCellFunctor<VertBF>(m_vertexIdArray, m_nVertices) );
					++m_nVertices;
				}
			}
			const HCTVertexOwnershipCursor& m_cursor;
			VertexIdArray& m_vertexIdArray;
			size_t& m_nVertices;
		};

		static inline size_t compute(Tree& tree, VertexIdArray& vertexIdArray)
		{
			tree.fitArray(&vertexIdArray);
			CellVertexIds defValue;
			defValue.fill( NotAVertexId );
			vertexIdArray.fill(defValue);
			size_t nbVertices = 0;

			// first pass : generate ids of cell owned vertices
			tree.preorderParseLeaves([&vertexIdArray,&nbVertices](const HCTVertexOwnershipCursor& cursor)
			{
				cursor.m_nbh.forEachVertexComponent(Pass1VertexFunctor(cursor,vertexIdArray,nbVertices ) );
			}
			, HCTVertexOwnershipCursor(tree));

			// 2nd pass : propagate up to the root
			tree.postorderParseCells([&tree,&vertexIdArray](const DefaultTreeCursor& cursor)
			{
				Cell cell = cursor.cell();
				if (!tree.isLeaf(cell))
				{
					GridDimension<D> grid = tree.getLevelSubdivisionGrid(cell.level());
					for (size_t v = 0; v < CellNumberOfVertices; v++)
					{
						Vec<size_t, D> cornerChildLocation = grid * hct::bitfield_vec<D>(v);
						Cell cornerChild = tree.child(cell, cornerChildLocation);
						vertexIdArray[cell][v] = vertexIdArray[cornerChild][v];
					}
				}
			});

			return nbVertices;
		}
	};
}

