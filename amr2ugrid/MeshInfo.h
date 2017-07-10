#ifndef __AMR_MESH_INFO_H
#define __AMR_MESH_INFO_H

/*---------------------------------------------------------
  Description d'un niveau de subdivision de l'arbre
---------------------------------------------------------*/
#include "Vec.h"
#include "AmrCellSize.h"
#include "PointIds.h"
#include "PathBits.h"

namespace Amr2Ugrid
{
	using namespace hct;
	
  template <unsigned int D>
    struct MeshConnectivity
    {
      using PointIds = hct::PointIds<D>;
      using Path = hct::Vec<bool,D>;

      int nCells;
      PointIds * pointIds;
      bool * enabled;

      inline MeshConnectivity() : nCells(0), pointIds(0), enabled(0) {}

      inline ~MeshConnectivity()
      {
	if( pointIds != 0 ) delete [] pointIds;
	if( enabled != 0 ) delete [] enabled;
      }
      
      template <typename StreamT>
      inline void toStream(StreamT& out) const
      {
	out<<nCells<<" cells\n";
	/*
	for(int i=0;i<nCells;i++)
	{
	  out<<"Cell["<<i<<"] = ("; pointIds[i].toStream(out); out<<") : en="<<enabled[i]<<"\n";
	}
	*/
      }

      template <typename StreamT>
      inline void toBinaryStream(StreamT& out) const
      {
	out.write( (char*) &nCells, sizeof(int) );
	out.write( (char*) pointIds, nCells*sizeof(PointIds) );
	out.write( (char*) enabled, nCells*sizeof(bool) );
      }

      template <typename StreamT>
      inline void fromBinaryStream(StreamT& in)
      {
	in.read( (char*) &nCells, sizeof(int) );
	pointIds = new PointIds[nCells];
	in.read( (char*) pointIds, nCells*sizeof(PointIds) );
	enabled = new bool[nCells];
	in.read( (char*) enabled, nCells*sizeof(bool) );
      }

      template<typename T>
      inline void init( const PointIds* indices, const hct::Vec<T,D>* points )
      {
	pointIds = new PointIds[ nCells ];
	enabled = new bool[ nCells ];

	for(int i=0;i<nCells;i++)
	  {
	    // calcul min/max et centre
	    hct::Vec<T,D> p0 = points[indices[i].nodes[0]];
	    hct::Vec<T,D> center(p0);
	    for(int j=1 ; j<PointIds::Size ; j++)
	      {
		hct::Vec<T,D> p = points[ indices[i].nodes[j] ];
		center += p;
	      }
	    center /= PointIds::Size;

	    // affectation des identifiants des points sur les sommets de l'hyper cube
	    pointIds[i] = PointIds(-1);
	    for(int j=0 ; j<PointIds::Size ; j++)
	      {
		const int index = indices[i].nodes[j];
		hct::Vec<T,D> p = points[ index ];
		Path path = ( p > center );
		unsigned int pi = PathBits<D>::fromPath( path );
		pointIds[i].nodes[ pi ] = index;
	      }
	    enabled[i] = ! pointIds[i].contains(-1);
	  }
      }
    };

  template<typename T, unsigned int D> struct MeshGeometry
  {
    using PointIds = hct::PointIds<D>;
    using Vec = hct::Vec<T,D>;
    using CellSize = hct::AmrCellSize<T,D>;

    int nPoints;
    Vec* points;

    inline MeshGeometry() : nPoints(0), points(0) {}

    inline ~MeshGeometry()
    {
      if( points != 0 ) delete [] points;
    }

    template<typename StreamT>
    inline void toStream(StreamT& out) const
    {
      out<<nPoints<<" points\n";
      /*
      for(int i=0;i<nPoints;i++)
	{
	  out<<"Point["<<i<<"] = ("; points[i].toStream(out); out<<")\n";
	}
      */
    }

    template<typename StreamT>
    inline void toBinaryStream(StreamT& out) const
    {
      out.write( (char*) &nPoints, sizeof(int) );
      out.write( (char*)points, sizeof(Vec)*nPoints );
    }

    template<typename StreamT>
    inline void fromBinaryStream(StreamT& in)
    {
      in.read( (char*) &nPoints, sizeof(int) );
      points = new Vec[nPoints];
      in.read( (char*)points, sizeof(Vec)*nPoints );
    }

    inline void computeCellCenters(const MeshConnectivity<D>& meshcon, Vec* centers)
    {
      for(int i=0;i<meshcon.nCells;i++)
	{
	  Vec p = points[meshcon.pointIds[i].nodes[0]];
	  for(int j=1 ; j<PointIds::Size ; j++)
	    {
	      p += points[ meshcon.pointIds[i].nodes[j] ];
	    }
	  p /= PointIds::Size;
	  centers[i] = p ;
	}
    }
    
    inline void computeCellSizes(const MeshConnectivity<D>& meshcon, CellSize* sizes)
    {
      for(int i=0;i<meshcon.nCells;i++)
	{
	  Vec p0 = points[ meshcon.pointIds[i].nodes[0] ];
	  Vec pmin(p0), pmax(p0); 
	  for(int j=1 ; j<PointIds::Size ; j++)
	    {
	      Vec p = points[ meshcon.pointIds[i].nodes[j] ];
	      pmin = pmin.min(p);
	      pmax = pmax.max(p);
	    }
	  sizes[i] = (pmax - pmin);
	}
    }
    
    inline void computeDomainBounds(const MeshConnectivity<D>& meshcon, Vec& bmin, Vec& bmax)
    {
      if( meshcon.nCells <= 0 ) return;
      bmin = bmax = points[ meshcon.pointIds[0].nodes[0] ];
      for(int i=0;i<meshcon.nCells;i++)
	{
	    for(int j=0 ; j<PointIds::Size ; j++)
	      {
		Vec p = points[ meshcon.pointIds[i].nodes[j] ];
		bmin = bmin.min(p);
		bmax = bmax.max(p);
	      }
	    
	}
    }
  };
  

}; // namespace Amr2Ugrid

#endif
