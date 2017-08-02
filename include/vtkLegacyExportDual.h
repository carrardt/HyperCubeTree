#pragma once

#include "Vec.h"
#include "HyperCubeTree.h"
#include "HyperCubeTreeVertexOwnershipCursor.h"
#include "HyperCubeTreeLocatedCursor.h"
#include "HyperCubeTreeDualMesh.h"
#include "TreeLevelStorage.h"

#include <vector>
#include <map>
#include <iostream>
#include <cstdint>

#include "std_array_ostream.h"

namespace hct
{
	namespace vtk
	{
		template<typename Tree>
		static inline void exportDualUnstructuredGrid(const Tree& tree, std::ostream& out)
		{
			static constexpr unsigned int D = Tree::D;
			using VecF = hct::Vec<double, D>;
			using VecI = hct::Vec<size_t, D>;
			constexpr size_t CellNumberOfVertices = 1 << Tree::D;
			using HCTVertexOwnershipCursor = hct::HyperCubeTreeVertexOwnershipCursor<Tree>;
			using HyperCubeTreeLocatedCursor = hct::HyperCubeTreeLocatedCursor<Tree>;
			using DefaultTreeCursor = typename Tree::DefaultTreeCursor;
			using DualMesh = hct::HyperCubeTreeDualMesh<Tree>;
			using DuallCell = DualMesh::DuallCell;

			assert(D >= 1 && D <= 3);

			out << "# vtk DataFile Version 2.0\n";
			out << "Exported from an HyperCubeTree object\n";
			out << "ASCII\n";
			out << "DATASET UNSTRUCTURED_GRID\n";

			size_t nLeaves = 0;
			tree.parseLeaves( [&nLeaves](const DefaultTreeCursor&) { ++nLeaves; } );

			out << "POINTS " << nLeaves << " double\n";

			TreeLevelArray<int64_t> leafIndex;
			tree.fitArray(&leafIndex);
			leafIndex.fill(-1);
			tree.parseLeaves( [&out, &nLeaves, &leafIndex](const HyperCubeTreeLocatedCursor& cursor)
			{
				leafIndex[cursor.cell()] = nLeaves;
				++nLeaves;
				cursor.position().normalize().toStream(out, " ");
			}
			, HyperCubeTreeLocatedCursor() );

			// number of dual cells is the number of primary vertices not on the boundary
			size_t numberOfCells = 0;
			tree.parseLeaves(
				[&numberOfCells](const HCTVertexOwnershipCursor& cursor)
				{
					constexpr size_t CellNumberOfVertices = 1 << Tree::D;
					for (size_t i = 0; i < CellNumberOfVertices; i++)
					{
						if (cursor.ownsVertex(i))
						{
							auto v = cursor.vertexPosition(i);
							if (!v.boundary())
							{
								++numberOfCells;
							}
						}
					}
				}
				, HCTVertexOwnershipCursor(tree) );

			// write cell connectivity
			out << "CELLS " << numberOfCells << ' ' << numberOfCells*(CellNumberOfVertices+1) << '\n';
			DualMesh::parseDualCells( tree,
				[&out,&leafIndex](const DuallCell& dual)
				{
					constexpr size_t CellNumberOfVertices = 1 << Tree::D;
					if (!dual.m_center.boundary())
					{
						out << CellNumberOfVertices;
						for (size_t i = 0; i < CellNumberOfVertices; i++)
						{
							assert(dual.m_vertices[i].m_cell.isTreeCell());
							assert(leafIndex[dual.m_vertices[i].m_cell] != -1);
							out << ' ' << leafIndex[dual.m_vertices[i].m_cell];
						}
					}
				});

			out << "CELL_TYPES " << numberOfCells << '\n';
			int cellType = -1;
			if (D == 1) { cellType = 1; /*VTK_VERTEX*/ }
			else if (D == 2) { cellType = 8; /*VTK_PIXEL*/ }
			else if (D == 3) { cellType = 11; /*VTK_VOXEL*/ }
			for (size_t i = 0; i < numberOfCells; i++)
			{
				out << cellType << '\n';
			}

			// only scalar fields are supported by now
			size_t nbArrays = tree.getNumberOfArrays();
			out << "POINT_DATA " << nLeaves << '\n';
			for (size_t a = 0; a < nbArrays; a++)
			{
				ITreeLevelArray* iarray = tree.array(a);
				out << "SCALARS " << iarray->name() << " float "<< iarray->numberOfComponents()<<"\nLOOKUP_TABLE default\n";
				tree.parseLeaves([&out,iarray](const typename Tree::DefaultTreeCursor & cursor)
				{
					iarray->toStream(out,cursor.cell());
					out << '\n';
				});
			}
		}
	}
}

