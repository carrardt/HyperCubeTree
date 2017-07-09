#include "MeshInfo.h"

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
