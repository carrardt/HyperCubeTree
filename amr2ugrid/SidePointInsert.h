#ifndef __SIDE_POINT_INSERT_H
#define __SIDE_POINT_INSERT_H

#ifdef DEBUG
#include <iostream>
#endif

#include "PointIds.h"
#include "CubeEnum.h"

namespace Amr2Ugrid
{

	using namespace hct;

  template<unsigned int _D> struct AmrSidePoints;
  template<unsigned int _D> struct AmrConnect;

  template<unsigned int _D,typename _Mask> struct SidePointCount
  {
    enum { D = _D };
    using Mask =_Mask;
    using PointIds = hct::PointIds<D>;
    using ElementInfo = typename Amr2Ugrid::AmrConnect<D>::ElementInfo;

    inline SidePointCount(AmrSidePoints<D>& s, int l, int n, const ElementInfo& _nbh )
      : self(s), level(l), node(n), nbh(_nbh) {}
    
    // not a functer 'cause it has a type specialization (Point)
    template<typename Point> inline void operator () ( Point )
    {
      if( node!=-1 && nbh.node!=-1 )
	{
	  bool ok = PointStatus<D>::isSidePoint( nbh.upSliding, nbh.downSliding, Mask(), Point() );
	  int myPoint = Point::BITFIELD;
	  int myPointId = self.pointIds[level][node][ myPoint ];
	  bool owner = ( myPointId >= 0 );
	  myPointId = owner ? myPointId : ((-myPointId)-1) ;
#ifdef DEBUG
	  int myCellId = nodeCellId[level][node];
	  int nbhCellId = nodeCellId[nbh.level][nbh.node];
#endif
	  if( ok )
	    {
	      if ( owner )
		{
#ifdef DEBUG
	      		if ( sidePointSet[nbh.level][nbh.node].find(myPointId)!=sidePointSet[nbh.level][nbh.node].end() )
			{
				std::cout<<'L'<<nbh.level<<".N"<<nbh.node<<".P"<<myPointId<<" ("; Mask::toStream(std::cout);
				std::cout<<") inserted more than once"<<std::endl;
			}
	      		sidePointSet[nbh.level][nbh.node].insert(myPointId);
#endif
		  	self.sidePoints[nbh.level][nbh.node] ++;
#ifdef DEBUG
			if( self.sidePoints[nbh.level][nbh.node] != sidePointSet[nbh.level][nbh.node].size() )
			{
				std::cout<<'L'<<nbh.level<<".N"<<nbh.node<<".P"<<myPointId<<" ("; Mask::toStream(std::cout);
				std::cout<<") NS="<<self.sidePoints[nbh.level][nbh.node]<<" != "
					 <<sidePointSet[nbh.level][nbh.node].size()<<" [ ";
				for(std::set<int>::iterator it=sidePointSet[nbh.level][nbh.node].begin();
					it!=sidePointSet[nbh.level][nbh.node].end();++it)
				{
					std::cout<<*it<<' ';
				}
				std::cout<<"]"<<std::endl;
			}
#endif
		}
	    }	      
	}
    }
    AmrSidePoints<D>& self;
    int level, node;
    const ElementInfo& nbh;
  };
  
  // operateur applique sur chaque sous-element d'un n-cube
  template<unsigned int _D> struct CountElementSidePoints
  {
    enum { D = _D };
    using PointIds = hct::PointIds<D> ;
    using ElementInfo = typename Amr2Ugrid::AmrConnect<D>::ElementInfo ;

    inline CountElementSidePoints(AmrSidePoints<D>& s) : self(s), level(0), node(-1) {}
    template<typename M> inline void processComponent(const CubeEnum<ElementInfo,0,M>& c)
    {
      if( CubeEnum<ElementInfo,0,M>::Mask::N_FREE > 0 && CubeEnum<ElementInfo,0,M>::Mask::N_FREE < CubeEnum<ElementInfo,0,M>::Mask::N_BITS )
	{
	  SidePointCount< D , typename CubeEnum<ElementInfo,0,M>::Mask > sidePointCount(self,level,node,c.value);
	  CubeEnum<ElementInfo,0,M>::Mask::enumerate( sidePointCount );
	}
    }
    AmrSidePoints<D>& self;
    int level,node;
  };

}; // Amr2Ugrid

#endif
/* ===================================================================================================================*/
