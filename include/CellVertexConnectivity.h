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
		using DefaultTreeCursor = typename Tree:DefaultTreeCursor;
		using HCTVertexOwnershipCursor = hct::HyperCubeTreeVertexOwnershipCursor<Tree>;
		using HCubeComponentValue = typename HCTVertexOwnershipCursor::HCubeComponentValue;
		static constexpr unsigned int D = Tree::D;
		static constexpr size_t CellNumberOfVertices = static_cast<size_t>(1) << D;
		using Cell = hct::HyperCubeTreeCell;
		static constexpr size_t NotAVertexId = std::numeric_limits<size_t>::max();
		using CellVertexIds = std::array<size_t, CellNumberOfVertices>;
		using VertexIdArray = TreeLevelArray<CellVertexIds>;

		static inline void compute(Tree& tree, VertexIdArray& vertexIdArray)
		{
			tree.fitArray(&vertexIdArray);
			vertexIdArray.fill(NotAVertexId);

			// first pass : generate ids of cell owned vertices
			size_t nbVertices = 0;
			tree.preorderParseLeaves([&vertexIdArray,&nbVertices](const HCTVertexOwnershipCursor& cursor)
			{
				for (size_t i = 0; i < CellNumberOfVertices; i++)
				{
					if (cursor.ownsVertex(i))
					{
						vertexIdArray[cursor.cell()] = nbVertices;
						++nbVertices;
						// copy to neighbor cells
					}
				}
			}
			, HCTVertexOwnershipCursor(tree));

			// 2nd pass : propagate up to the root
			tree.postorderParseCells([&tree,&vertexIdArray](const DefaultTreeCursor& cursor)
			{

			});


		}
	};
}

