#pragma once

#include "HyperCubeTreeCell.h"
#include "NumericalValueTraits.h"
#include "ITreeLevelArray.h"
#include "TreeLevelArray.h"

#include <cstdint>
#include <vector>
#include <cstring>
#include <assert.h>
#include <iostream>
#include <string>

namespace hct
{

	class TreeLevelStorage
	{
		public:
			inline size_t getNumberOfLevels() const
			{
				return m_level_sizes.size();
			}

			inline void setNumberOfLevels(size_t n)
			{
				m_level_sizes.resize(n,0);
				for (auto a : m_level_arrays) { a->setNumberOfLevels(n); }
			}

			inline void resize(size_t level, size_t nElems)
			{
				assert( level < getNumberOfLevels() );
				m_level_sizes[level] = nElems;
				for (auto a : m_level_arrays) { a->resize(level,nElems); }
			}

			inline size_t getLevelSize(size_t level) const
			{
				assert(level < getNumberOfLevels());
				return m_level_sizes[level];
			}

			inline bool checkArraySizes() const
			{
				for (auto a : m_level_arrays)
				{
					for (size_t l = 0; l < getNumberOfLevels(); l++)
					{
						assert(a->size(l) == getLevelSize(l));
					}
				}
				return true;
			}

			inline void erase(size_t level, size_t position, size_t nElems)
			{
				assert( level < getNumberOfLevels() );
				assert( (position + nElems) <= m_level_sizes[level] );
				m_level_sizes[level] -= nElems;
				for (auto a : m_level_arrays) { a->erase(level, position, nElems); }
			}

			// does not actually add the array, but resizes it so that it fits the level sizes
			inline void fitArray(ITreeLevelArray* a) const
			{
				a->setNumberOfLevels(getNumberOfLevels());
				for (size_t i = 0; i < getNumberOfLevels(); i++)
				{
					a->resize(i, getLevelSize(i) );
				}
			}

			inline size_t addArray(ITreeLevelArray* a)
			{
				fitArray(a);
				m_level_arrays.push_back(a);
				return m_level_arrays.size() - 1;
			}

			inline size_t getNumberOfArrays() const
			{
				return m_level_arrays.size();
			}

			inline ITreeLevelArray* array(size_t i) const
			{
				assert(i<getNumberOfArrays());
				return m_level_arrays[i];
			}

			template<typename StreamT>
			inline StreamT& toStream(StreamT & out)
			{
				out << "Number of arrays : " << m_level_arrays.size() << '\n';
				out << "Number of levels : " << m_level_sizes.size() << '\n';
				size_t totalSize = 0;
				for (size_t i = 0; i < getNumberOfLevels(); i++)
				{
					size_t levelSize = m_level_sizes[i];
					out << "\tLevel " << i << " : size = " << levelSize << '\n';
					totalSize += levelSize;
				}
				out << "Total size : " << totalSize << '\n';
				return out;
			}

		private:
			std::vector<size_t> m_level_sizes;
			std::vector< ITreeLevelArray* > m_level_arrays;
	};

}
