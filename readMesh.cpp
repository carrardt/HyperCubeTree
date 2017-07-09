#include <iostream>
#include <fstream>
using namespace std;

#include "readMesh.h"

bool readMesh(const char* fileName, int& nc, int& np, int*& indices, AmrReconstruction3::Vec3f*& points, float*& scalars)
{
	ifstream fic(fileName);
	if( !fic.good() ) return false;

	// lecture du nombre de mailles et nimbre de points
	fic.read( (char*)&nc, sizeof(int) ); // nombre de mailles
	fic.read( (char*)&np, sizeof(int) ); // nombre de points

	indices = new int[8*nc];
	points = new AmrReconstruction3::Vec3f[np];
	scalars = new float[nc];

	// lecture du maillage
	for(int i=0;i<nc;i++)
	{
		int ncp;
		fic.read( (char*)&ncp , sizeof(int) );
		if(ncp==8) // on verifie que c'est un tetra
		{
		  fic.read( (char*)(indices+i*8) , 8*sizeof(int) );
		}
		else // on saute la maille foireuse
		{
		  cout<<"erreur dans la maille "<<i<<endl;
		  for(int j=0;j<8;j++) indices[i*8+j] = -1;
		  fic.ignore(ncp*sizeof(int));
		}
	}
	fic.read( (char*)points , np*sizeof(AmrReconstruction3::Vec3f) );
	fic.read( (char*)scalars , nc*sizeof(float) );

	fic.close();
	return true;
}

#ifdef _TEST_readMesh
int main(int argc, char* argv[])
{
  // verification du nombre minimal d'arguments
  if( argc<2 )
    {
      cerr<<"Utilisation: "<<argv[0]<<" <nom_du_fichier> [-c] [-p] [-s]"<<endl;
      return 1;
    }

  // lecture des options
  bool printCells=false;
  bool printPoints=false;
  bool printScalars=false;
  for(int i=2;i<argc;i++)
    {
      if( strcmp(argv[i],"-c")==0 ) printCells=true;
      if( strcmp(argv[i],"-p")==0 ) printPoints=true;
      if( strcmp(argv[i],"-s")==0 ) printScalars=true;
    }

  // lecture et affichage du maillage
  int nc=0;
  int np=0;
  int* indices=0;
  AmrReconstruction3::Vec3f* points=0;
  float* scalars=0;

  if( readMesh(argv[1],nc,np,indices,points,scalars) )
    {
      cout<<nc<<" cells"<<endl;
      cout<<np<<" points"<<endl;
      if(printCells) for(int i=0;i<nc;i++)
	{
	  cout<<"Cell["<<i<<"] = ("<<indices[i*8];
	  for(int j=0;j<8;j++) cout<<','<<indices[i*8+j];
	  cout<<endl;
	}
      if(printPoints) for(int i=0;i<np;i++)
	{
	  cout<<"Point["<<i<<"] = ("; points[i].toStream(cout); cout<<')'<<endl;
	}
      if(printScalars) for(int i=0;i<nc;i++)
	{
	  cout<<"Scalar["<<i<<"] = "<<scalars[i]<<endl;
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

