#ifndef __POINT_CONNECT_H
#define __POINT_CONNECT_H

#include "Vec.h"
#include "PointStatus.h"

namespace Amr2Ugrid
{

	using namespace hct;

  template<unsigned int _D> struct AmrSidePoints;
  template<unsigned int _D> struct AmrConnect;

  template<unsigned int _D,typename _Mask> struct PointConnect
  {
    enum { D = _D };
    using Mask = _Mask;
    using PointIds = hct::PointIds<D> ;
    using ElementInfo = typename Amr2Ugrid::AmrConnect<D>::ElementInfo ;

    inline PointConnect(AmrSidePoints<D>& s, int l, int n, const ElementInfo& _nbh )
      :  self(s), level(l), node(n), nbh(_nbh) {}
    
    template<typename Point> inline void operator () ( Point )
    {
      if( node!=-1 && nbh.node!=-1 )
	{
	  int myPoint = Point::BITFIELD;
	  int nbhPoint = Point::BITFIELD ^ Mask::DEF_BITFIELD ;
	  int myPointId = self.pointIds[level][node][myPoint];
	  int nbhPointId = self.pointIds[nbh.level][nbh.node][nbhPoint];
	  bool ok = PointStatus<D>::isPointFacing(nbh.upSliding,nbh.downSliding, Point());
#ifdef DEBUG
	  int myCellId = nodeCellId[level][node];
	  int nbhCellId = nodeCellId[nbh.level][nbh.node];
	  if( myCellId!=-1 && nbhCellId!=-1 )
	    {
	      int myMeshPoint = mesh.pointIds[myCellId][myPoint];
	      int nbhMeshPoint = mesh.pointIds[nbhCellId][nbhPoint];
	      Vec<float,3> p1 = geom.points[myMeshPoint];
	      Vec<float,3> p2 = geom.points[nbhMeshPoint];
	      bool geomMatch = ( (p2-p1).length2() == 0.0 );
	      if( ( ok && !geomMatch ) || ( !ok && geomMatch ) )
		{
		  std::cout<<"isPointFacing = "<<ok<<std::endl;
		  std::cout<<'l'<<level<<".n"<<node<<'.'; Mask::toStream(std::cout); std::cout<<".c"<<myCellId<<".p"<<myPoint<<" <-> l";
		  std::cout<<nbh.level<<".n"<<nbh.node<<"."; Mask::Negate::toStream(std::cout); std::cout<<".c"<<nbhCellId<<".p"<<nbhPoint<<std::endl;
		  std::cout<<"UpSliding   : "; nbh.upSliding.toStream(std::cout); std::cout<<std::endl;
		  std::cout<<"DownSliding : "; nbh.downSliding.toStream(std::cout); std::cout<<std::endl;
		  std::cout<<"difference  : "; (p2-p1).toStream(std::cout); std::cout<<std::endl<<std::endl;
		}
	      /*else if( level != nbh.level )
		{
		  std::cout<<level<<' '<<nbh.level<<std::endl;
		  }*/
	    }
#endif
	  if( ok )
	    {
	      // on utilise le > pour ne pas avoir de probleme avec 0
	      // si 0 != 1, alors on affecte -1 a la place de 0, donc pas d'ambiguite
	      if( nbhPointId < myPointId ) 
		{
		  //std::cout<<myPointId<<" <- "<<nbhPointId<<std::endl;
		  self.pointIds[level][node][myPoint] = nbhPointId;
		}
	      else 
		{
		  //std::cout<<nbhPointId<<" <- "<<myPointId<<std::endl;
		  self.pointIds[nbh.level][nbh.node][nbhPoint] = myPointId;
		}
	    }
	}
    }
    AmrSidePoints<D>& self;
    int level, node;
    const ElementInfo& nbh;
  };
  
  // operateur applique sur chaque sous-element d'un cube
  template<unsigned int _D> struct ElementConnect
  {
    enum { D = _D };
    using PointIds = Amr2Ugrid::PointIds<D>;
    using ElementInfo = typename Amr2Ugrid::AmrConnect<D>::ElementInfo;

    inline ElementConnect(AmrSidePoints<D>& s) : self(s), level(0), node(-1) {}
    template<typename M> inline void processComponent(const CubeEnum<ElementInfo,0,M>& c)
    {
      if( CubeEnum<ElementInfo,0,M>::Mask::N_FREE < CubeEnum<ElementInfo,0,M>::Mask::N_BITS )
	{
	  PointConnect< D , typename CubeEnum<ElementInfo,0,M>::Mask > pointConnect(self,level,node,c.value);
	  CubeEnum<ElementInfo,0,M>::Mask::enumerate( pointConnect );
	}
    }
    AmrSidePoints<D>& self;
    int level,node;
  };

}; // Amr2Ugrid

#endif
/* ===================================================================================================================*/

