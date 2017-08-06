#pragma once

#include "HyperCubeTree.h"
#include "HyperCubeTreeLocatedCursor.h"
#include "Vec.h"

namespace hct
{
		template<typename Tree, typename FuncT>
		static inline void tree_refine_implicit_surface(Tree & tree, FuncT f, size_t maxLevel)
		{
			using TreeCursor = HyperCubeTreeLocatedCursor<Tree>;
			using T = typename FuncT::T;
			static constexpr unsigned int D = Tree::D;
			using VecT = hct::Vec<T, D>;

			tree.preorderParseCells(
			[f, maxLevel, &tree](TreeCursor cursor)
			{
				if (tree.isRefinable(cursor.cell()) && cursor.cell().level() < maxLevel)
				{
					constexpr size_t nVertices = 1 << TreeCursor::D;
					bool allInside = true;
					bool allOutside = true;
					VecT normal;
					bool sameDirection = true;
					for (size_t i = 0; i < nVertices; i++)
					{
						VecT x = cursor.vertexPosition(i).normalize();
						auto Fx = f(x);
						if (allInside && allOutside)
						{
							normal = Fx.gradient();
						}
						else if (normal.dot(Fx.gradient()) < 0.0)
						{
							sameDirection = false;
						}
						if (Fx.value() > 0.0) { allInside = false; }
						else { allOutside = false; }
					}
					if ((!allInside && !allOutside) || !sameDirection)
					{
						tree.refine(cursor.cell());
					}
				}
			}
			, TreeCursor() );
		}



}