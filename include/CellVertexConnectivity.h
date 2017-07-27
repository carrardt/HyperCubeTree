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
			inline Pass1VertexNeighborCellFunctor(const HCTVertexOwnershipCursor& cursor, VertexIdArray& vertexIdArray, size_t vertexId)
				: m_cursor(cursor)
				, m_vertexIdArray(vertexIdArray)
				, m_vertexId(vertexId)
			{}

			template<typename T, typename CompBF>
			inline void operator() (const T& neighbor, CompBF)
			{
				if (CompBF::N_DEF > 0) // neighbor is not myself
				{
					using NeighborVertBF = typename NeighborVertex<CompBF, VertBF>::Vertex;
					constexpr size_t originalVertex = VertBF::BITFIELD;
					constexpr size_t neighborVertex = NeighborVertBF::BITFIELD;
					Cell originalCell = m_cursor.cell();
					Cell neighborCell = neighbor.cell();
					if (neighborCell.isTreeCell() && neighborCell.level() == originalCell.level())
					{
						size_t prevVertexId = m_vertexIdArray[neighborCell][neighborVertex];
						assert(prevVertexId == NotAVertexId || prevVertexId == m_vertexId);
						m_vertexIdArray[neighborCell][neighborVertex] = m_vertexId;
					}
				}
			}
			const HCTVertexOwnershipCursor& m_cursor;
			VertexIdArray& m_vertexIdArray;
			size_t m_vertexId;
		};


		struct Pass1aVertexFunctor
		{
			inline Pass1aVertexFunctor(const HCTVertexOwnershipCursor& cursor, VertexIdArray& vertexIdArray, size_t& nVertices)
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
					Cell meCell = m_cursor.cell();
					assert(m_vertexIdArray[meCell][i] == NotAVertexId);
					m_vertexIdArray[meCell][i] = m_nVertices;
					m_cursor.m_nbh.forEachComponentSharingVertex(VertBF(), Pass1VertexNeighborCellFunctor<VertBF>(m_cursor,m_vertexIdArray, m_nVertices));
					++m_nVertices;
				}
			}
			const HCTVertexOwnershipCursor& m_cursor;
			VertexIdArray& m_vertexIdArray;
			size_t& m_nVertices;
		};

		struct Pass1bVertexFunctor
		{
			inline Pass1bVertexFunctor(const HCTVertexOwnershipCursor& cursor, VertexIdArray& vertexIdArray)
				: m_cursor(cursor)
				, m_vertexIdArray(vertexIdArray)
			{}

			template<typename T, typename VertBF>
			inline void operator() (const T& value, VertBF)
			{
				constexpr size_t i = VertBF::BITFIELD;
				size_t vertexId = m_vertexIdArray[m_cursor.cell()][i];
				if (vertexId != NotAVertexId)
				{
					m_cursor.m_nbh.forEachComponentSharingVertex(VertBF(), Pass1VertexNeighborCellFunctor<VertBF>(m_cursor, m_vertexIdArray, vertexId));
				}
			}
			const HCTVertexOwnershipCursor& m_cursor;
			VertexIdArray& m_vertexIdArray;
		};

		static inline size_t compute(Tree& tree, VertexIdArray& vertexIdArray)
		{
			tree.fitArray(&vertexIdArray);
			CellVertexIds defValue;
			defValue.fill( NotAVertexId );
			vertexIdArray.fill(defValue);
			size_t nbVertices = 0;

			/* first pass :
				- generate ids for owned vertices
				- propagate to neighbors at the same level
				- propagate up to parent
			*/
			tree.postorderParseCells([&tree,&vertexIdArray,&nbVertices](const HCTVertexOwnershipCursor& cursor)
			{
				Cell cell = cursor.cell();
				if ( tree.isLeaf(cell) )
				{
					cursor.m_nbh.forEachVertexComponent(Pass1aVertexFunctor(cursor, vertexIdArray, nbVertices));
				}
				else
				{
					GridDimension<D> maxLocation = tree.getLevelSubdivisionGrid(cell.level()) - 1;
					for (size_t v = 0; v < CellNumberOfVertices; v++)
					{
						Vec<size_t, D> cornerChildLocation = maxLocation * hct::bitfield_vec<D>(v);
						Cell cornerChildCell = tree.child(cell, cornerChildLocation);
						size_t childVertexId = vertexIdArray[cornerChildCell][v];
						if (childVertexId != NotAVertexId)
						{
#							ifndef NDEBUG
							size_t prevVertId = vertexIdArray[cell][v];
							assert(prevVertId == NotAVertexId || prevVertId == childVertexId);
#							endif
							vertexIdArray[cell][v] = childVertexId;
						}
					}
					cursor.m_nbh.forEachVertexComponent(Pass1bVertexFunctor(cursor, vertexIdArray));
				}
			}
			, HCTVertexOwnershipCursor(tree));

			return nbVertices;
		}
	};
}

