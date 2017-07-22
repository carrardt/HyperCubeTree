#pragma once

#include "Vec.h"
#include "HyperCubeTree.h"
#include "HyperCubeTreeVertexOwnershipCursor.h"

#include <vector>

namespace hct
{
	namespace vtk
	{
		template<typename Tree, typename StreamT>
		static inline void exportUnstructuredGrid(const Tree& tree, StreamT& out)
		{
			static constexpr unsigned int D = Tree::D;
			using VecT = hct::Vec<double, D>;
			using HCTVertexOwnershipCursor = hct::HyperCubeTreeVertexOwnershipCursor<Tree>;

			out << "# vtk DataFile Version 2.0\n";
			out << "Exported from an HyperCubeTree object\n";
			out << "ASCII\n";
			out << "DATASET UNSTRUCTURED_GRID\n";


			std::vector<VecT> vertices;
			Vec3d domainBounds = tree.getLevelSubdivisionGrid(0);
			tree.preorderParseCells( [&vertices,domainBounds](const HCTVertexOwnershipCursor& cursor)
			{
				using HCubeComponentValue = typename HCTVertexOwnershipCursor::HCubeComponentValue;
				constexpr size_t CellNumberOfVertices = 1 << Tree::D;
				for (size_t i = 0; i < CellNumberOfVertices; i++)
				{
					if (cursor.ownsVertex(i))
					{
						auto vertex = hct::bitfield_vec<Tree::D>(i);
						Vec3d p = cursor.position() + vertex;
						p *= domainBounds / cursor.resolution();
						vertices.push_back(p);
					}
				}
			}
			, HCTVertexOwnershipCursor(tree) );

			out << "POINTS " << vertices.size() << " double\n";
			for (VecT p : vertices)
			{
				p.toStream(out, " ");
				out << '\n';
			}
		}
	}
}

