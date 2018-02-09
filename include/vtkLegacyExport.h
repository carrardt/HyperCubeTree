#pragma once

#include "Vec.h"
#include "HyperCubeTree.h"
#include "HyperCubeTreeVertexOwnershipCursor.h"
#include "CellVertexConnectivity.h"

#include <vector>
#include <map>
#include <iostream>

#include "std_array_ostream.h"

namespace hct
{
	namespace vtk
	{
		template<typename Tree>
		static inline void exportUnstructuredGrid(const Tree& tree, std::ostream& out)
		{
			static constexpr unsigned int D = Tree::D;
			constexpr size_t CellNumberOfVertices = 1 << Tree::D;
			using PositionI = hct::Vec<size_t, D>;
			using PositionF = hct::Vec<double, D>;
			using HCTVertexOwnershipCursor = hct::HyperCubeTreeVertexOwnershipCursor<Tree>;
			using CellVertexConnectivity = hct::CellVertexConnectivity<Tree>;
			using CellVertexIds = typename CellVertexConnectivity::CellVertexIds;
			using VertexIdArray = typename CellVertexConnectivity::VertexIdArray;

			assert(D >= 1 && D <= 3);

			out << "# vtk DataFile Version 2.0\n";
			out << "Exported from an HyperCubeTree object\n";
			out << "ASCII\n";
			out << "DATASET UNSTRUCTURED_GRID\n";

			// build connectivity
			VertexIdArray vertexIds;
			size_t nVertices = CellVertexConnectivity::compute(tree, vertexIds);
			std::vector< HyperCubeTreeCell > leaves;

			out << "POINTS " << nVertices << " double\n";
			tree.parseLeaves( [&out, &leaves](const HCTVertexOwnershipCursor& cursor)
			{
				using HCubeComponentValue = typename HCTVertexOwnershipCursor::HCubeComponentValue;
				constexpr size_t CellNumberOfVertices = 1 << Tree::D;
				for (size_t i = 0; i < CellNumberOfVertices; i++)
				{
					if (cursor.ownsVertex(i))
					{
						auto vertex = hct::bitfield_vec<Tree::D>(i);
						(cursor.position() + vertex).normalize().toStream(out, " ");
						out << '\n';
					}
				}
				leaves.push_back( cursor.cell() );
			}
			, HCTVertexOwnershipCursor(tree) );

			// write cell connectivity
			size_t numberOfCells = leaves.size();
			out << "CELLS " << numberOfCells << ' ' << numberOfCells*(CellNumberOfVertices+1) << '\n';
			for (auto cell : leaves)
			{
				out << CellNumberOfVertices;
				for (size_t i = 0; i < CellNumberOfVertices; i++)
				{
					out << ' ' << vertexIds[cell][i];
				}
				out << '\n';
			}

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
			out << "CELL_DATA " << numberOfCells << '\n';
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

