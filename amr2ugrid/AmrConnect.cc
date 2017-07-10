#include "AmrConnect.h"

#include <iostream>
#include <fstream>
#include <string>
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
  string baseName = argv[1];
  string levelFileName = baseName+".lvl";
  string treeFileName = baseName+".tree";
  string nbhFileName = baseName+".nbh";

  cout<<"<- "<<levelFileName<<endl;
  ifstream fic_grid( levelFileName.c_str() );
  if( !fic_grid )
    {
      cerr<<"Erreur lecture "<<levelFileName<<endl;
      return 1;
    }
  AmrLevels<float,3> levels;
  levels.fromBinaryStream(fic_grid);
  fic_grid.close();
  levels.toStream(cout); cout<<endl;

  cout<<"<- "<<treeFileName<<endl;
  ifstream fic_tree( treeFileName.c_str() );
  if( !fic_tree )
    {
      cerr<<"Erreur lecture "<<treeFileName<<endl;
      return 1;
    }
  AmrTree tree;
  tree.fromBinaryStream(fic_tree);
  fic_tree.close();
  tree.toStream(cout); cout<<endl;

  AmrConnect<3> amrConnect(tree,levels.levelInfo);
  amrConnect.connectTree();
  amrConnect.toStream(cout);

  cout<<"-> "<<nbhFileName<<endl;
  ofstream fic_nbh(nbhFileName.c_str());
  if( !fic_nbh )
    {
      cout<<"Erreur ecriture "<<nbhFileName<<endl;
      return 1;
    }
  amrConnect.toBinaryStream(fic_nbh);
  fic_nbh.close();

  return 0;
}
