#pragma once

#include "Vec.h"
#include "HyperCubeTree.h"
#include "HyperCubeTreeVertexOwnershipCursor.h"
#include "CellVertexConnectivity.h"

#include <vector>
#include <map>
#include <iostream>

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
			using CellVertexIds = CellVertexConnectivity::CellVertexIds;
			using VertexIdArray = CellVertexConnectivity::VertexIdArray;

			assert(D >= 1 && D <= 3);

			out << "# vtk DataFile Version 2.0\n";
			out << "Exported from an HyperCubeTree object\n";
			out << "ASCII\n";
			out << "DATASET UNSTRUCTURED_GRID\n";

			int nSubdivs = tree.getNumberOfLevelSubdivisions();
			std::vector< PositionI > resolutionMultiplier( nSubdivs+1 , PositionI(1) );
			for (int i = 0; i < nSubdivs ; i++)
			{
				resolutionMultiplier[i] = tree.getLevelSubdivisionGrid(i);
			}
			for (int i = nSubdivs-1; i >= 0; i--)
			{
				resolutionMultiplier[i] *= resolutionMultiplier[i+1];
			}

			// build connectivity
			VertexIdArray vertexIds;
			size_t nVertices = CellVertexConnectivity::compute(tree, vertexIds);
			std::vector< HyperCubeTreeCell > leaves;

			out << "POINTS " << nVertices << " double\n";
			tree.preorderParseLeaves( [&leaves, &resolutionMultiplier](const HCTVertexOwnershipCursor& cursor)
			{
				using HCubeComponentValue = typename HCTVertexOwnershipCursor::HCubeComponentValue;
				constexpr size_t CellNumberOfVertices = 1 << Tree::D;
				for (size_t i = 0; i < CellNumberOfVertices; i++)
				{
					if (cursor.ownsVertex(i))
					{
						auto vertex = hct::bitfield_vec<Tree::D>(i);
						PositionI p = cursor.position() + vertex;
						p *= resolutionMultiplier[cursor.cell().level()];
						PositionF pcoord = p;
						pcoord /= resolutionMultiplier[0];
						pcoord.toStream(out, " ");
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
				out << "SCALARS " << iarray->name() << " float 1\nLOOKUP_TABLE default\n";
				tree.preorderParseLeaves([&out,iarray](const typename Tree::DefaultTreeCursor & cursor)
				{
					iarray->toStream(out,cursor.cell());
					out << '\n';
				});
			}
		}
	}
}

