#ifndef __TREE_CONNECT_H
#define __TREE_CONNECT_H

#include "AmrTree.h"
#include "HyperCube.h"

namespace AmrReconstruction3
{

  template<typename T, unsigned int D> struct TreeConnect
  {
    typedef CubeEnum<T,D> Nbh;

    static inline void dig( const Nbh<int,D>& parent, const LevelInfo& level, const Vec<int,D>& coord, Nbh<int,D>&result )
    {
      
    }

    static inline void parseCellPairs( AmrTree& tree, CellInfo& cell, LevelInfo* levels )
    {
      Nbh<int,D> nbh(-1);
    }
  };

}; // namespace AmrReconstruction3

#endif

