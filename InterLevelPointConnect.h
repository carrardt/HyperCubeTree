#ifndef __INTER_LEVEL_POINT_CONNECT
#define __INTER_LEVEL_POINT_CONNECT

#include "Vec.h"
#include "PointIds.h"

namespace AmrReconstruction3
{

  template<unsigned int DecD, unsigned int IncD=0, typename Point=NullBitField> struct InterLevelPointConnect;
  template<unsigned int D, typename Point> struct InterLevelPointConnect<0,D,Point>
  {
    static inline void connect( PointIds<D>& parentIds, PointIds<D>* childIds, const Vec<unsigned int,D>& grid, Vec<unsigned int,D> coord)
    {
      int point = Point::Reverse::BITFIELD;
      int branch = grid.gridIndex( coord.reverse() );
      int pid = parentIds[point];
      int cid = childIds[branch][point];
      if( pid < cid )
	{
	  childIds[branch][point] = pid;
	}
      else
	{
	  parentIds[point] = cid;
	}
    }
  };
  template<unsigned int DecD, unsigned int IncD, typename Point> struct InterLevelPointConnect
  {
    enum { D = DecD+IncD };
    typedef Vec<unsigned int,D> Grid_D;
    typedef Vec<unsigned int,IncD> Grid_IncD;

    static inline void connect( PointIds<D>& parentIds, PointIds<D>* childIds,
				const Grid_D& grid, 
				Grid_IncD gridHead = Grid_IncD() )
    {
      unsigned int gridVal = grid.Vec<unsigned int,DecD>::val;

      InterLevelPointConnect< DecD-1, IncD+1, CBitField<Bit0,Point> >
	::connect( parentIds,childIds,grid, Vec<unsigned int,IncD+1>(0,gridHead) );

      InterLevelPointConnect< DecD-1, IncD+1, CBitField<Bit1,Point> >
	::connect(parentIds,childIds,grid, Vec<unsigned int,IncD+1>(gridVal-1,gridHead) );
					  
    }
  };

}; // AmrReconstruction3

#endif
/* ===================================================================================================================*/
