#include "readMesh.h"

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
