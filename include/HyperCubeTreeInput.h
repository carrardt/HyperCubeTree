#pragma once

#include <string>
#include <vector>
#include <memory>

#include "ScalarFunctionInput.h"
#include "SimpleSubdivisionScheme.h"
#include "TreeRefineImplicitSurface.h"
#include "TreeLevelStorage.h"
#include "HyperCubeTreeLocatedCursor.h"

namespace hct
{
	template<unsigned int D, typename T, typename StreamT>
	static HyperCubeTree<D, SimpleSubdivisionScheme<D> > 
	inline read_tree(StreamT& input,
					std::vector< std::shared_ptr<hct::TreeLevelArray<T> > >& scalars,
					std::vector< std::shared_ptr<hct::TreeLevelArray<Vec<T,D> > > >& vectors )
	{
		using Tree = HyperCubeTree<D, SimpleSubdivisionScheme<D> >;
		using TreeCursor = typename Tree::DefaultTreeCursor;
		using LocatedTreeCursor = HyperCubeTreeLocatedCursor<Tree>;

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

		Tree tree(levels);

		input >> token;
		if (token == "refine")
		{
			while (token == "refine")
			{
				input >> token;
				if (token == "level")
				{
					size_t l = 0;
					input >> l;
					assert(l >= 0 && l < tree.getNumberOfLevels());
					tree.parseLeaves([&tree,l](const TreeCursor& cursor)
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
		}
		else if (token == "structure")
		{
			assert("'structure' keyword not handled" && false);
		}
		else
		{
			abort();
		}

		while (token == "scalar" || token == "gradient" /*|| token=="vector"*/ )
		{
			std::string name, functionOrData;
			input >> name;
			input >> functionOrData;
			if (token == "scalar")
			{
				std::shared_ptr<TreeLevelArray<T> > scalarField( new TreeLevelArray<T>() );
				scalarField->setName(name);
				tree.addArray(scalarField.get());
				if (functionOrData == "function")
				{
					auto f = scalar_function_read<D, T>(input);
					tree.preorderParseCells( [scalarField,f](const LocatedTreeCursor& cursor)
					{
						(*scalarField)[cursor.cell()] = f( cursor.position().addHalfUnit().normalize() ).value();
					}, LocatedTreeCursor() );
				}
				else // if(functionOrData=="data")
				{
					abort();
				}
				scalars.push_back(scalarField);
			}
			else if( token=="gradient" )
			{
				std::shared_ptr<TreeLevelArray< Vec<T,D> > > vectorField( new TreeLevelArray< Vec<T, D> >() );
				vectorField->setName(name);
				tree.addArray(vectorField.get());
				if (functionOrData == "function")
				{
					auto f = scalar_function_read<D, T>(input);
					tree.preorderParseCells([vectorField, f](const LocatedTreeCursor& cursor)
					{
						(*vectorField)[cursor.cell()] = f(cursor.position().addHalfUnit().normalize()).gradient();
					}, LocatedTreeCursor());
				}
				else // if(functionOrData=="data")
				{
					abort();
				}
			}
			else // if(token=="vector")
			{
				abort();
			}
			input >> token;
		}

		if (token == "end")
		{
			// finalize actions here ?
		}
		else
		{
			abort();
		}

		return tree;
	}

}