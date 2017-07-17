#pragma once

#include "HyperCubeTreeCell.h"

#include <cstdint>
#include <vector>
#include <cstring>
#include <assert.h>

namespace hct
{

	class ITreeLevelArray
	{
		public:
			virtual void setNumberOfLevels(size_t nLevels) =0;
			virtual void resize(size_t level, size_t nElems) =0;
			virtual void erase(size_t level, size_t position, size_t nElems) =0;
	};

	template<typename T>
	class TreeLevelArray : public ITreeLevelArray
	{
		using ElementReference = typename std::vector<T>::reference;
		using ConstElementReference = typename std::vector<T>::const_reference;
		public:
			inline void setNumberOfLevels(size_t nLevels) override final
			{
				m_arrays.resize(nLevels);
			}
			
			inline void resize(size_t level, size_t nElems) override final
			{
				assert( level<m_arrays.size() );
				m_arrays[level].resize(nElems);
			}
			
			inline void erase(size_t level, size_t position, size_t nElems) override final
			{
				assert( level<m_arrays.size() );				
				assert( (position+nElems) <= m_arrays[level].size() );
				m_arrays[level].erase( m_arrays[level].begin()+position, m_arrays[level].begin()+position+nElems );
			}
			
			inline const std::vector<T>& operator [] (size_t level) const
			{
				assert( level < m_arrays.size() );
				return m_arrays[level];
			}

			inline std::vector<T>& operator [] (size_t level)
			{
				assert( level < m_arrays.size() );
				return m_arrays[level];
			}

			inline ConstElementReference operator [] (HyperCubeTreeCell cell) const
			{
				assert(cell.level() < m_arrays.size());
				assert(cell.index() < m_arrays[cell.level()].size());
				return m_arrays[cell.level()][cell.index()];
			}

			inline ElementReference operator [] (HyperCubeTreeCell cell)
			{
				assert(cell.level() < m_arrays.size());
				assert(cell.index() < m_arrays[cell.level()].size());
				return m_arrays[cell.level()][cell.index()];
			}

			template<typename StreamT>
			inline StreamT& toStream (StreamT & out)
			{
				out << "Number of levels : " << m_arrays.size() << '\n';
				for (size_t i = 0; i < m_arrays.size(); i++)
				{
					out << "\tLevel " << i << " : size = " << m_arrays[i].size() << '\n';
				}
				return out;
			}

		private:
			std::vector< std::vector<T> > m_arrays;
	};

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

			inline void erase(size_t level, size_t position, size_t nElems)
			{
				assert( level < getNumberOfLevels() );
				assert( (position + nElems) <= m_level_sizes[level] );
				m_level_sizes[level] -= nElems;
				for (auto a : m_level_arrays) { a->erase(level, position, nElems); }
			}

			inline void addArray(ITreeLevelArray* a)
			{
				a->setNumberOfLevels(getNumberOfLevels());
				for (size_t i = 0; i < getNumberOfLevels(); i++)
				{
					a->resize(i,m_level_sizes[i]);
				}
				m_level_arrays.push_back(a);
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
