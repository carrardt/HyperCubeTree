#include "AmrTree.h"
#include "AmrLevels.h"

#include <iostream>
#include <fstream>
#include <string>
using namespace std;
using namespace hct;

int main(int argc, char* argv[])
{
  // verification du nombre minimal d'arguments
  if( argc<3 )
    {
      cerr<<"Utilisation: "<<argv[0]<<" fichier.lvl fichier.tree"<<endl;
      return 1;
    }
  string levelFileName = argv[1];
  string treeFileName = argv[2];

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

  return 0;
}
