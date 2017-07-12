#ifndef __AMR_LEVELS_H
#define __AMR_LEVELS_H

#include "Vec.h"
#include "AmrCellSize.h"
#include "GridDimension.h"

#include <map>

namespace Amr2Ugrid
{

	using namespace hct;

  template<unsigned int D>
    struct LevelInfo
    {
      GridDimension<D> grid;
      int depth;
      int nCells;
      inline LevelInfo() : grid(), depth(-1), nCells(0) {}
    };

  template <typename T,unsigned int D>
    struct AmrLevels
    {
		using AmrCellSize = hct::AmrCellSize<T,D>;
        using Grid = hct::GridDimension<D>;
		using LevelInfo = Amr2Ugrid::LevelInfo<D> ;

      typedef std::map<AmrCellSize,LevelInfo> LevelMap;     
      typedef typename LevelMap::iterator LevelMapIterator;
      typedef typename LevelMap::const_iterator LevelMapCIterator;
 
      inline AmrLevels() : nLevels(0), levelInfo(0), levelSize(0) {}

      inline ~AmrLevels()
      {
	if( levelInfo != 0 ) delete [] levelInfo;
	if( levelSize != 0 ) delete [] levelSize;
      }

      inline void fromMap( LevelMap& levelMap )
      {
	LevelMapIterator it = levelMap.begin();
	LevelMapIterator pit = levelMap.end();
	LevelMapIterator nit ;

	LevelMap normalizedLevelMap;
	AmrCellSize psize;

	unsigned int depth = 0;

	while(it!=levelMap.end())
	  {
	    nit = it; ++nit;

	    AmrCellSize size;
	    if( pit != levelMap.end() )
	      {
		size = psize / pit->second.grid;
	      }
	    else
	      {
		size = it->first;
	      }

	    if( nit != levelMap.end() )
	      {
		it->second.grid = ( size / nit->first ) + 0.5 ;
	      }
	    normalizedLevelMap[size] = it->second;
	    normalizedLevelMap[size].depth = depth++;

	    psize = size;
	    pit = it;
	    it = nit;
	  }

	levelMap = normalizedLevelMap;

	nLevels = levelMap.size();
	levelInfo = new LevelInfo[nLevels];
	levelSize = new AmrCellSize[nLevels];
	int i=0;
	for(LevelMapIterator it=levelMap.begin(); it!=levelMap.end(); ++it)
	  {
	    levelInfo[i] = it->second;
	    levelSize[i] = it->first;
	    i++;
	  }
      }

      template <typename StreamT> inline 
      void toStream(StreamT & out) const
      {
	for(int i=0;i<nLevels;i++)
	  {
	    out<<i<<" : (";
	    levelInfo[i].grid.toStream(out);
	    out<<")="<<levelInfo[i].grid.gridSize()<<" : (";
	    levelSize[i].toStream(out);
	    out<<") : depth="<<levelInfo[i].depth<<", nCells="<<levelInfo[i].nCells<<"\n";
	  }
      }

      template <typename StreamT> inline 
      void toBinaryStream(StreamT & out) const
      {
	out.write( (char*) &nLevels, sizeof(int) );
	out.write( (char*) levelInfo, sizeof(LevelInfo)*nLevels );
	out.write( (char*) levelSize, sizeof(AmrCellSize)*nLevels );
      }

      template <typename StreamT> inline 
      void fromBinaryStream(StreamT & in)
      {
	in.read( (char*) &nLevels, sizeof(int) );
	levelInfo = new LevelInfo[nLevels];
	levelSize = new AmrCellSize[nLevels];	
	in.read( (char*) levelInfo, sizeof(LevelInfo)*nLevels );
	in.read( (char*) levelSize, sizeof(AmrCellSize)*nLevels );
      }

      size_t nLevels;
      LevelInfo* levelInfo;
      AmrCellSize* levelSize;
    };

}; // namespace hct

#endif
