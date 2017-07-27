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

			/*for (int i = 0; i < (nSubdivs+1); i++)
			{
				std::cout << "res mult " << i << " = " << resolutionMultiplier[i] << '\n';
			}
			std::cout.flush();*/

			std::map<PositionI, int64_t> vertices;
			tree.preorderParseLeaves( [&vertices,&resolutionMultiplier](const HCTVertexOwnershipCursor& cursor)
			{
				using HCubeComponentValue = typename HCTVertexOwnershipCursor::HCubeComponentValue;
				constexpr size_t CellNumberOfVertices = 1 << Tree::D;
				for (size_t i = 0; i < CellNumberOfVertices; i++)
				{
					auto vertex = hct::bitfield_vec<Tree::D>(i);
					PositionI p = cursor.position() + vertex;
					p *= resolutionMultiplier[ cursor.cell().level() ];
					if (cursor.ownsVertex(i))
					{
						vertices[p] = -1;
					}
				}
			}
			, HCTVertexOwnershipCursor(tree) );

			out << "POINTS " << vertices.size() << " double\n";
			size_t vertexCount = 0;
			for (auto& it : vertices)
			{
				PositionF p = it.first;
				p /= resolutionMultiplier[0];
				p.toStream(out, " ");
				it.second = vertexCount;
				++vertexCount;
				out << '\n';
			}
			assert(vertexCount == vertices.size());
			for (auto it : vertices)
			{
				assert(it.second!=-1);
			}

			// build cell->vertices connectivity
			std::vector<size_t> cellVertices;
			tree.preorderParseLeaves([&cellVertices, &vertices, &resolutionMultiplier](const HCTVertexOwnershipCursor& cursor)
			{
				using HCubeComponentValue = typename HCTVertexOwnershipCursor::HCubeComponentValue;
				constexpr size_t CellNumberOfVertices = 1 << Tree::D;
				for (size_t i = 0; i < CellNumberOfVertices; i++)
				{
					auto vertex = hct::bitfield_vec<Tree::D>(i);
					PositionI p = cursor.position() + vertex;
					p *= resolutionMultiplier[cursor.cell().level()];
					int64_t vertexIndex = vertices[p];
					assert(vertexIndex != -1);
					cellVertices.push_back(vertexIndex);
				}
			}
			, HCTVertexOwnershipCursor(tree));

			// write cell connectivity
			size_t numberOfCells = cellVertices.size() / CellNumberOfVertices;
			out << "CELLS " << numberOfCells << ' ' << numberOfCells*(CellNumberOfVertices+1) << '\n';
			for (size_t i = 0; i < numberOfCells; i++)
			{
				out << CellNumberOfVertices;
				for (size_t j = 0; j < CellNumberOfVertices; j++)
				{
					out << ' ' << cellVertices[i*CellNumberOfVertices + j];
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

