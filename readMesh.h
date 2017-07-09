#ifndef __READ_MESH_H
#define __READ_MESH_H

#include "Vec.h"
#include "PointIds.h"

#include <fstream>
#include <string>

namespace AmrReconstruction3
{

  static inline bool readMesh(const char* fileName,
			      int& nc, int& np,
			      PointIds<3>*& indices,
			      Vec3f*& points,
			      float*& scalars)
  {
    std::ifstream fic(fileName);
    if( !fic.good() ) return false;

    // lecture du nombre de mailles et nimbre de points
    fic.read( (char*)&nc, sizeof(int) ); // nombre de mailles
    fic.read( (char*)&np, sizeof(int) ); // nombre de points

    indices = new PointIds<3>[nc];
    points = new Vec3f[np];
    scalars = new float[nc];

    // lecture du maillage
    for(int i=0;i<nc;i++)
      {
	int ncp;
	fic.read( (char*)&ncp , sizeof(int) );
	if(ncp==PointIds<3>::Size) // on verifie que c'est un tetra
	  {
	    fic.read( (char*)(indices+i) , sizeof(PointIds<3>) );
	  }
	else // on saute la maille foireuse
	  {
	    for(int j=0;j<8;j++) indices[i].nodes[j] = -1;
	    fic.ignore(ncp*sizeof(int));
	  }
      }
    fic.read( (char*)points , np*sizeof(Vec3f) );
    fic.read( (char*)scalars , nc*sizeof(float) );

    fic.close();
    return true;
  }

}; // namespace AmrReconstruction3

#endif //__AMR_VEC_H
/* ============================================================================== */


#ifdef _TEST_readMesh

#include <iostream>
using namespace std;
using namespace AmrReconstruction3;

int main(int argc, char* argv[])
{
	// verification du nombre minimal d'arguments
	if( argc<2 )
    {
      cerr<<"Utilisation: "<<argv[0]<<" <nom_du_fichier> [ [-a] | [-c] [-p] [-s] ]"<<endl;
      return 1;
    }

	// lecture des options
	bool printCells=false;
	bool printPoints=false;
	bool printScalars=false;
	for(int i=2;i<argc;i++)
	{
		if( std::string(argv[i]) == "-c" ) printCells=true;
		if( std::string(argv[i]) == "-p" ) printPoints=true;
		if( std::string(argv[i]) == "-s" ) printScalars=true;
		if( std::string(argv[i]) == "-a" )
		{
		  printCells=true;
		  printPoints=true;
		  printScalars=true;
		}
	}

  // lecture et affichage du maillage
  int nc=0;
  int np=0;
  PointIds<3>* indices=0;
  Vec3f* points=0;
  float* scalars=0;

  if( readMesh(argv[1],nc,np,indices,points,scalars) )
    {
      cout<<nc<<" cells"<<endl;
      cout<<np<<" points"<<endl;
      if(printCells) for(int i=0;i<nc;i++)
	{
	  cout<<"Cell["<<i<<"] = ("; indices[i].toStream(cout); cout<<")\n";
	}
      if(printPoints) for(int i=0;i<np;i++)
	{
	  cout<<"Point["<<i<<"] = ("; points[i].toStream(cout); cout<<")\n";
	}
      if(printScalars) for(int i=0;i<nc;i++)
	{
	  cout<<"Scalar["<<i<<"] = "<<scalars[i]<<'\n';
	}
      delete [] scalars;
      delete [] indices;
      delete [] points;
      return 0;
    }
  else
    {
      cerr<<"Erreur lecture"<<endl;
      return 1;
    }
}
#endif

