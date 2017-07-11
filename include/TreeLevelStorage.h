#pragma once

#include <cstdint>
#include <vector>
#include <cstring>
#include <assert.h>

namespace hct
{

	class ITreeLevelArray
	{
		public:
			virtual void setNumberOfLevels(unsigned int nLevels) =0;
			virtual void resize(unsigned int level, size_t nElems) =0;
			virtual void erase(unsigned int level, size_t position, size_t nElems) =0;
	};

	template<typename T>
	class TreeLevelArray : public ITreeLevelArray
	{
		public:
		
			inline void setNumberOfLevels(unsigned int nLevels) override final
			{
				m_arrays.resize(nLevels);
			}
			
			inline void resize(unsigned int level, size_t nElems) override final
			{
				assert( level<m_arrays.size() );
				m_arrays[level].resize(nElems);
			}
			
			inline void erase(unsigned int level, size_t position, size_t nElems) override final
			{
				assert( level<m_arrays.size() );				
				assert( (position+nElems) <= m_arrays[level].size() );
				m_arrays[level].erase( m_arrays[level].begin()+position, m_arrays[level].begin()+position+nElems );
			}
			
			inline const std::vector<T>& operator [] (unsigned int level) const
			{
				assert( level < m_arrays.size() );
				return m_arrays[level];
			}

			inline std::vector<T>& operator [] (unsigned int level)
			{
				assert( level < m_arrays.size() );
				return m_arrays[level];
			}
			
		private:
			std::vector< std::vector<T> > m_arrays;
	};


	class TreeLevelStorage
	{
		public:
			

		private:
			std::vector< ITreeLevelArray* > m_level_arrays;
	};
	
}
