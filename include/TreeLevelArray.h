#pragma once

#include "HyperCubeTreeCell.h"
#include "ITreeLevelArray.h"

#include <cstdlib>
#include <string>
#include <vector>
#include <iostream>
#include <assert.h>

namespace hct
{

	template<typename T>
	class TreeLevelArray : public ITreeLevelArray
	{
		using ElementReference = typename std::vector<T>::reference;
		using ConstElementReference = typename std::vector<T>::const_reference;
		public:

			/*virtual ~TreeLevelArray()
			{
				std::cout << "TreeLevelArray<T> " << name() << "being deleted\n";
			}*/

			inline void setName(const std::string& name)
			{
				m_name = name;
			}

			inline std::string name() const override final
			{
				return m_name;
			}

			inline void setNumberOfLevels(size_t nLevels) override final
			{
				m_arrays.resize(nLevels);
			}
			
			inline size_t numberOfLevels() const override final
			{
			  return m_arrays.size();
			}
			
			inline size_t size(size_t level) const override final
			{
				assert(level<m_arrays.size());
				return m_arrays[level].size();
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
			
			inline void fill(const T& value)
			{
				for (auto& a : m_arrays) for (auto& x : a) { x = value; }
			}

			inline std::ostream& printCell(std::ostream& out, HyperCubeTreeCell cell) const override final
			{
				out << m_arrays[cell.level()][cell.index()];
				return out;
			}

			size_t numberOfComponents() const override final
			{
				return NumericalValueTraits<T>::NumberOfComponents;
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

		private:
			std::vector< std::vector<T> > m_arrays;
			std::string m_name;
	};

}


