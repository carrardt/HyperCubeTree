#pragma once

#include "Vec.h"
#include "GridDimension.h"

namespace hct
{

	// ==================== StaticGrid =======================
	/* A type holding a list of integers, representing the dimensions of a grid
	*/
	template<unsigned int... S>
	struct StaticGridDim
	{
		static constexpr unsigned int D = sizeof...(S);
		static inline Vec<unsigned int,D> value()
		{
			return Vec<unsigned int, D>({ S... });
		}
	};

	template<typename... GridTypes>
	struct StaticGridDimSet
	{
		static constexpr size_t grid_size = 0;
		static constexpr bool is_valid = false;
	};
	template<unsigned int... S>
	struct StaticGridDimSet< StaticGridDim<S...> >
	{
		static constexpr size_t grid_size = sizeof...(S);
		static constexpr bool is_valid = true;
		using HeadGrid = StaticGridDim<S...>;
	};

	template<unsigned int... S, typename... GridTypes>
	struct StaticGridDimSet< StaticGridDim<S...> , GridTypes... >
	{
		static constexpr size_t grid_size = sizeof...(S);
		static constexpr bool is_valid = StaticGridDimSet<GridTypes...>::is_valid && StaticGridDimSet<GridTypes...>::grid_size==grid_size;
		using HeadGrid = StaticGridDim<S...>;
	};

	template<size_t _NLevels, unsigned int... S>
	struct StaticSubdivisionScheme
	{
		static constexpr size_t NLevels = _NLevels;
		using GridDim = StaticGridDim<S...>;
		static constexpr unsigned int D = GridDim::D;

		static constexpr size_t getNumberOfLevelSubdivisions() 
		{
			return NLevels;
		}

		static constexpr GridDimension<D> getLevelSubdivision(size_t) 
		{
			return GridDimension<D>(GridDim::value());
		}

		template<typename StreamT>
		StreamT& toStream(StreamT & out)
		{
			for (size_t i = 0; i < getNumberOfLevelSubdivisions(); i++)
			{
				out << "subdivision " << i << " : "; getLevelSubdivision(i).toStream(out); out << '\n';
			}
			return out;
		}
	};

	template<typename T1, typename T2, bool is_valid=(T1::D==T2::D) >
	struct StaticSubdivisionSchemeCombo {};

	template<typename T1, typename T2>
	struct StaticSubdivisionSchemeCombo<T1,T2,true>
	{
		static constexpr size_t NLevels = T1::NLevels + T2::NLevels;
		static constexpr unsigned int D = T1::D;
		static constexpr size_t getNumberOfLevelSubdivisions() 
		{
			return NLevels;
		}
		static constexpr GridDimension<D> getLevelSubdivision(size_t level) 
		{
			return (level < T1::NLevels) ? T1::getLevelSubdivision(level) : T2::getLevelSubdivision(level - T1::NLevels);
/*			if (level < T1::NLevels) return T1::getLevelSubdivision(level);
			else return T2::getLevelSubdivision(level-T1::NLevels); */
		}

		template<typename StreamT>
		StreamT& toStream(StreamT & out)
		{
			for (size_t i = 0; i < getNumberOfLevelSubdivisions(); i++)
			{
				out << "subdivision " << i << " : "; getLevelSubdivision(i).toStream(out); out << '\n';
			}
			return out;
		}
	};


}
