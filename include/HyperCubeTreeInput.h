#pragma once

#include <string>

#include "ScalarFunctionInput.h"
#include "SimpleSubdivisionScheme.h"
#include "TreeRefineImplicitSurface.h"
#include "TreeLevelStorage.h"

namespace hct
{
	template<unsigned int D, typename T, typename StreamT>
	static HyperCubeTree<D, SimpleSubdivisionScheme<D> > 
	inline read_tree(StreamT& input)
	{ 
		std::string token;
		input >> token;
		if (token == "dimension")
		{
			unsigned int nd = 0;
			input >> nd;
			assert(nd == D);
		}
		else
		{
			abort();
		}

		SimpleSubdivisionScheme<D> levels;
		input >> token;
		if (token == "levels")
		{
			levels.fromStream(input);
		}
		else 
		{ 
			abort(); 
		}

		using Tree = HyperCubeTree<D, SimpleSubdivisionScheme<D> >;
		using TreeCursor = typename Tree::DefaultTreeCursor;
		Tree tree;

		input >> token;
		while (token == "refine")
		{
			input >> token;
			if (token == "level")
			{
				size_t l = 0;
				input >> l;
				assert(l >= 0 && l < tree.getNumberOfLevels());
				tree.parseLeaves( [l](const TreeCursor& cursor)
				{
					if (tree.isRefinable(cursor.cell()) && cursor.cell().level() == l)
					{
						tree.refine(cursor.cell());
					}
				});
			}
			else if (token == "surface")
			{
				size_t maxLevel = 0;
				input >> maxLevel;
				auto surf = scalar_function_read<D, T>(input);
				tree_refine_implicit_surface(tree, surf, maxLevel);
			}
			else
			{
				abort();
			}
			// the may be several successive refine directives
			input >> token;
		}

		while (token == "scalar" || token == "gradient" /*|| token=="vector"*/ )
		{
			std::string name;
			input >> name;
			if (token == "scalar")
			{

			}
			else if( token=="gradient" )
			{

			}
			else
			{
				abort();
			}

			input >> token;
		}
	}

}