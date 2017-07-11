#include "Vec.h"
#include "AmrLevels.h"
#include "AmrTree.h"

#include <iostream>
#include <fstream>
using namespace std;

using namespace Amr2Ugrid;

int main(int argc, char* argv[])
{
  // verification du nombre minimal d'arguments
  if( argc<2 )
    {
      cerr<<"Utilisation: "<<argv[0]<<" nom_du_test"<<endl;
      return 1;
    }  

  string testName = argv[1];
  string centersFileName = testName + ".cc"; // input
  string depthFileName = testName + ".dpt";  // input
  string levelsFileName = testName + ".lvl"; // input
  string boundsFileName = testName + ".bnd"; // input
  string treeFileName = testName + ".tree";  // output
  string m2tFileName = testName + ".m2t";    // output

  cout<<"<- "<<centersFileName<<endl;
  ifstream fic_cc(centersFileName.c_str());
  if( !fic_cc )
    {
      cerr<<"Erreur lecture "<<centersFileName<<endl;
      return 1;
    }
  int nCells = 0;
  Vec<float,3> * cellCenters;
  fic_cc.read( (char*)&nCells, sizeof(int) );
  cellCenters = new Vec<float,3>[nCells];
  fic_cc.read( (char*)cellCenters, sizeof(Vec<float,3>)*nCells );
  fic_cc.close();

  cout<<"<- "<<levelsFileName<<endl;
  ifstream fic_lvl(levelsFileName.c_str());
  if( !fic_lvl )
    {
      cerr<<"Erreur lecture "<<levelsFileName<<endl;
      return 1;
    }
  AmrLevels<float,3> levels;
  levels.fromBinaryStream(fic_lvl);
  fic_lvl.close();
  levels.toStream(cout); cout<<endl;

  cout<<"<- "<<depthFileName<<endl;
  ifstream fic_dpt(depthFileName.c_str());
  if( !fic_dpt )
    {
      cerr<<"Erreur lecture "<<depthFileName<<endl;
      return 1;
    }
  int * cellDepth = 0;
  fic_dpt.read( (char*)&nCells, sizeof(int) );
  cellDepth = new int [ nCells ];
  fic_dpt.read( (char*)cellDepth, sizeof(int)*nCells );

  cout<<"<- "<<boundsFileName<<endl;
  ifstream fic_bnd(boundsFileName.c_str());
  if( !fic_bnd )
    {
      cerr<<"Erreur lecture "<<boundsFileName<<endl;
      return 1;
    }
  Vec<float,3> bmin, bmax;
  fic_bnd.read( (char*)&bmin, sizeof(Vec<float,3>) );
  fic_bnd.read( (char*)&bmax, sizeof(Vec<float,3>) );
  fic_bnd.close();
  
  // initialisation arbre
  AmrTree tree;
  tree.initLevels( levels.nLevels, levels.levelInfo );
  tree.toStream(cout); cout<<endl;

  // allocation d'un tableau pour recuperer la correspondance maillage -> arbre
  int * m2t = new int [ nCells ];

  // insertion des mailles dans l'arbre
  for(int i=0;i<nCells;i++)
    {
      m2t[i] = tree.insertCell( cellCenters[i], cellDepth[i], bmin, levels.levelInfo, levels.levelSize );
    }
  tree.toStream(cout); cout<<endl;
  delete [] cellCenters;
  delete [] cellDepth;

  cout<<"-> "<<m2tFileName<<endl;
  ofstream fic_m2t(m2tFileName.c_str());
  if( !fic_m2t )
    {
      cerr<<"Erreur ecriture "<<m2tFileName<<endl;
      return 1;
    }
  fic_m2t.write( (char*)&nCells, sizeof(int) );
  fic_m2t.write( (char*)m2t, sizeof(int)*nCells );
  fic_m2t.close();
  delete [] m2t;
  
  cout<<"-> "<<treeFileName<<endl;
  ofstream fic_tree(treeFileName.c_str());
  if( !fic_tree )
    {
      cerr<<"Erreur ecriture "<<treeFileName<<endl;
      return 1;
    }
  tree.toBinaryStream(fic_tree);
  fic_tree.close();

  return 0;
}
