#ifndef __AMR_MESH_INFO_H
#define __AMR_MESH_INFO_H

/*---------------------------------------------------------
  Description d'un niveau de subdivision de l'arbre
---------------------------------------------------------*/
#include "Vec.h"
#include "AmrCellSize.h"
#include "PointIds.h"
#include "PathBits.h"

namespace AmrReconstruction3
{
  template <unsigned int D>
    struct MeshConnectivity
    {
      typedef PointIds<D> PointIds;
      typedef Vec<bool,D> Path;

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
      inline void init( const PointIds* indices, const Vec<T,D>* points )
      {
	pointIds = new PointIds[ nCells ];
	enabled = new bool[ nCells ];

	for(int i=0;i<nCells;i++)
	  {
	    // calcul min/max et centre
	    Vec<T,D> p0 = points[indices[i].nodes[0]];
	    Vec<T,D> center(p0);
	    for(int j=1 ; j<PointIds::Size ; j++)
	      {
		Vec<T,D> p = points[ indices[i].nodes[j] ];
		center += p;
	      }
	    center /= PointIds::Size;

	    // affectation des identifiants des points sur les sommets de l'hyper cube
	    pointIds[i] = PointIds(-1);
	    for(int j=0 ; j<PointIds::Size ; j++)
	      {
		const int index = indices[i].nodes[j];
		Vec<T,D> p = points[ index ];
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
    typedef PointIds<D> PointIds;
    typedef Vec<T,D> Vec;
    typedef AmrCellSize<T,D> CellSize;

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
  

}; // namespace AmrReconstruction3

#endif
/* ===================================================================================================================*/



#ifdef _TEST_MeshInfo
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

#include "readMesh.h"
using namespace AmrReconstruction3;

int main(int argc, char* argv[])
{
  // verification du nombre minimal d'arguments
  if( argc<2 )
    {
      cerr<<"Utilisation: "<<argv[0]<<" nom_du_test"<<endl;
      return 1;
    }
  
  string testName = argv[1];
  string binFileName = testName + ".bin";
  string meshFileName = testName + ".con";
  string geomFileName = testName + ".geom";
  string scalarFileName = testName + ".scal";
  string boundsFileName = testName + ".bnd";
  string centersFileName = testName + ".cc";
  string sizesFileName = testName + ".cs";

  MeshConnectivity<3> mesh;
  MeshGeometry<float,3> geom;
  
  PointIds<3>* indices=0;
  float* scalars=0;

  cout<<"<- "<<binFileName<<endl;
  if( readMesh(binFileName.c_str(),mesh.nCells,geom.nPoints,indices,geom.points,scalars) )
    {
      ofstream fic;

      mesh.init(indices,geom.points);
      delete [] indices;
      mesh.toStream(cout);
      geom.toStream(cout);

      fic.open(scalarFileName.c_str());
      cout<<"-> "<<scalarFileName<<endl;
      fic.write( (char*)&(mesh.nCells), sizeof(int) );
      fic.write( (char*)scalars, sizeof(float)*mesh.nCells );
      fic.close();
      delete [] scalars;

      fic.open(meshFileName.c_str());
      cout<<"-> "<<meshFileName<<endl;
      mesh.toBinaryStream(fic);
      fic.close();

      fic.open(geomFileName.c_str());
      cout<<"-> "<<geomFileName<<endl;
      geom.toBinaryStream(fic);
      fic.close();

      Vec<float,3> bmin,bmax;
      geom.computeDomainBounds(mesh,bmin,bmax);
      cout<<"Domain bounds : ("; bmin.toStream(cout); cout<<") - ("; bmax.toStream(cout); cout<<")\n";
      fic.open(boundsFileName.c_str());
      cout<<"-> "<<boundsFileName<<endl;
      fic.write( (char*)&bmin, sizeof(Vec<float,3>) );
      fic.write( (char*)&bmax, sizeof(Vec<float,3>) );
      fic.close();

      Vec<float,3>* centers = new Vec<float,3>[mesh.nCells];
      geom.computeCellCenters(mesh,centers);
/*       cout<<"Cell centers :\n"; */
/*       for(int i=0;i<mesh.nCells;i++) */
/* 	{ */
/* 	  centers[i].toStream(cout); cout<<endl; */
/* 	} */
      fic.open(centersFileName.c_str());
      cout<<"-> "<<centersFileName<<endl;
      fic.write( (char*)&(mesh.nCells), sizeof(int) );
      fic.write( (char*)centers, sizeof(Vec<float,3>)*mesh.nCells );
      fic.close();
      delete [] centers;
      
      AmrCellSize<float,3>* sizes = new AmrCellSize<float,3>[mesh.nCells];
      geom.computeCellSizes(mesh,sizes);
/*       cout<<"Cell sizes :\n"; */
/*       for(int i=0;i<mesh.nCells;i++) */
/* 	{ */
/* 	  sizes[i].toStream(cout); cout<<endl; */
/* 	} */
      fic.open(sizesFileName.c_str());
      cout<<"-> "<<sizesFileName<<endl;
      fic.write( (char*)&(mesh.nCells), sizeof(int) );
      fic.write( (char*)sizes, sizeof(AmrCellSize<float,3>)*mesh.nCells );
      fic.close();
      delete [] sizes;

      return 0;
    }
  else
    {
      cerr<<"Erreur de lecture de "<<binFileName<<endl;
      return 1;
    }
}

#endif


