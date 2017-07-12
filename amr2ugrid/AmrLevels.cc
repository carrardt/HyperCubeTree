#include <iostream>
#include <fstream>
#include <string>
using namespace std;

#include "AmrLevels.h"
#include "MeshInfo.h"
using namespace Amr2Ugrid;
using namespace hct;

int main(int argc, char* argv[])
{
  // verification du nombre minimal d'arguments
  if( argc<2 )
    {
      cerr<<"Utilisation: "<<argv[0]<<" nom_du_test"<<endl;
      return 1;
    }  

  string testName = argv[1];
  string boundsFileName = testName + ".bnd"; // input
  string sizesFileName = testName + ".cs";   // input
  string levelsFileName = testName + ".lvl"; // output
  string depthFileName = testName + ".dpt";  // output

  cout<<"<- "<<sizesFileName<<endl;
  AmrCellSize<float,3> * cellSizes = 0;
  int nCells = 0;
  ifstream fic_sizes(sizesFileName.c_str());
  if(!fic_sizes)
    {
      cerr<<"Erreur lecture "<<sizesFileName<<endl;
      return 1;
    }
  fic_sizes.read( (char*)&nCells, sizeof(int) );
  cellSizes = new AmrCellSize<float,3>[nCells];
  fic_sizes.read( (char*)cellSizes, sizeof(AmrCellSize<float,3>)*nCells );
  fic_sizes.close();

  // construction des niveaux
  AmrLevels<float,3> levels;
  AmrLevels<float,3>::LevelMap levelMap;

  for(int i=0;i<nCells;i++)
    {
      levelMap[ cellSizes[i] ].nCells++;
    }

  cout<<"<- "<<boundsFileName<<endl;
  Vec<float,3> bmin,bmax;
  ifstream fic_bounds(boundsFileName.c_str());
  if(!fic_bounds)
    {
      cerr<<"Erreur lecture "<<boundsFileName<<endl;
      return 1;
    }
  fic_bounds.read( (char*)&bmin, sizeof(Vec<float,3>) );
  fic_bounds.read( (char*)&bmax, sizeof(Vec<float,3>) );
  fic_bounds.close();

  // finalisation des niveaux
  levelMap[ bmax - bmin ].nCells++;
  levels.fromMap(levelMap);
  levels.toStream(cout);

  // recuperation de la profondeur en fonction de la taille
  int * depth = new int [nCells];
  for(int i=0;i<nCells;i++)
    {
      depth[i] = levelMap[ cellSizes[i] ].depth;
    }
  delete [] cellSizes;
  cout<<"-> "<<depthFileName<<endl;
  ofstream fic_depth(depthFileName.c_str());
  if( !fic_depth )
    {
      cerr<<"Erreur ecriture "<<depthFileName<<endl;
      return 1;
    }
  fic_depth.write( (char*)&nCells, sizeof(int) );
  fic_depth.write( (char*)depth, sizeof(int)*nCells );
  fic_depth.close();
  delete [] depth;
  levelMap.clear();

  cout<<"-> "<<levelsFileName<<endl;
  ofstream fic_lvl(levelsFileName.c_str());
  if( !fic_lvl )
    {
      cerr<<"Erreur ecriture "<<levelsFileName<<endl;
      return 1;      
    }
  levels.toBinaryStream(fic_lvl);

  return 0;
}
