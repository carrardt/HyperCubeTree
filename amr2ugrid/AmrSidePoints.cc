#include "AmrSidePoints.h"

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
using namespace std;
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

  string baseName = argv[1];
  string levelFileName = baseName+".lvl";    // input
  string treeFileName = baseName+".tree";    // input
  string nbhFileName = baseName+".nbh";      // input

#ifdef DEBUG
  cout<<"--- DEBUG ---"<<endl;
  string geomFileName = baseName + ".geom";
  string meshFileName = baseName + ".con";  
  string m2tFileName = baseName + ".m2t";   
  string depthFileName = baseName + ".dpt"; 
  string centersFileName = baseName + ".cc";
  string sizesFileName = baseName + ".cs";
  string levelsFileName = baseName + ".lvl";
  ifstream fic_dbg;

  cout<<"<- "<<meshFileName<<endl;
  fic_dbg.open( meshFileName.c_str() );
  mesh.fromBinaryStream(fic_dbg);
  fic_dbg.close();

  cout<<"<- "<<geomFileName<<endl;
  fic_dbg.open( geomFileName.c_str() );
  geom.fromBinaryStream(fic_dbg);
  fic_dbg.close();

  cout<<"<- "<<m2tFileName<<endl;
  fic_dbg.open(m2tFileName.c_str());
  int nCells = 0;
  fic_dbg.read((char*)&nCells,sizeof(int));
  assert( nCells == mesh.nCells );
  cellNode = new int [nCells];
  fic_dbg.read((char*)cellNode,sizeof(int)*nCells);
  fic_dbg.close();

  cout<<"<- "<<depthFileName<<endl;
  fic_dbg.open(depthFileName.c_str());
  fic_dbg.read((char*)&nCells,sizeof(int));
  assert( nCells == mesh.nCells );
  cellDepth = new int [nCells];
  fic_dbg.read((char*)cellDepth,sizeof(int)*nCells);
  fic_dbg.close();

  cout<<"<- "<<centersFileName<<endl;
  fic_dbg.open(centersFileName.c_str());
  fic_dbg.read((char*)&nCells,sizeof(int));
  assert( nCells == mesh.nCells );
  cellCenter = new Vec<float,3> [nCells];
  fic_dbg.read((char*)cellCenter,sizeof(Vec<float,3>)*nCells);
  fic_dbg.close();

  cout<<"<- "<<sizesFileName<<endl;
  fic_dbg.open(sizesFileName.c_str());
  fic_dbg.read((char*)&nCells,sizeof(int));
  assert( nCells == mesh.nCells );
  cellSize = new AmrCellSize<float,3> [nCells];
  fic_dbg.read((char*)cellSize,sizeof(AmrCellSize<float,3>)*nCells);
  fic_dbg.close();

  cout<<"<- "<<levelsFileName<<endl;
  fic_dbg.open(levelsFileName.c_str());
  levels.fromBinaryStream(fic_dbg);
  fic_dbg.close();

  mesh.toStream(cout);
  geom.toStream(cout);
  cout<<"-------------"<<endl;
#endif
  
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
  
#ifdef DEBUG
  nodeCellId = new int*[tree.nLevels];
  for(int i=0;i<tree.nLevels;i++)
    {
      nodeCellId[i] = new int[tree.nodeLevels[i].size];
      for(int j=0;j<tree.nodeLevels[i].size;j++) nodeCellId[i][j] = -1;
    }
  for(int i=0;i<mesh.nCells;i++)
    {
      int d = cellDepth[i];
      int n = cellNode[i];
      nodeCellId[d][n] = i;
    }

  sidePointSet = new std::set<int>*[ tree.nLevels ];
  for(int i=0;i<tree.nLevels;i++)
    {
      int ncubes = tree.nodeLevels[i].size;
      sidePointSet[i] = new std::set<int>[ ncubes ];
    }
#endif

  cout<<"<- "<<nbhFileName<<endl;
  ifstream fic_nbh( nbhFileName.c_str() );
  if( !fic_nbh )
    {
      cerr<<"Erreur lecture "<<nbhFileName<<endl;
      return 1;
    }
  AmrConnect<3> amrConnect(tree,levels.levelInfo);
  amrConnect.fromBinaryStream(fic_nbh);
  fic_nbh.close();
  amrConnect.toStream(cout);

#ifdef PROFILING
   auto T1 = std::chrono::high_resolution_clock::now();
#endif

  AmrSidePoints<3> amrSidePoints;
  amrSidePoints.init(tree);
  for(int i=0;i<3;i++)
    {
      amrSidePoints.unifyPoints( tree, levels.levelInfo, amrConnect );
    }
  amrSidePoints.markPointOwners( tree );
  amrSidePoints.countSidePoints( tree, amrConnect );
  amrSidePoints.restorePointIdMap( tree );
  amrSidePoints.buildSidePointArray( tree, amrConnect );

#ifdef PROFILING
  auto T2 = std::chrono::high_resolution_clock::now();
  auto usec = std::chrono::duration_cast<std::chrono::microseconds>(T2 - T1);
   cout << "Elapsed time = " << usec.count() <<" uSec"<<endl;
#endif


  cout<<"Nb points unifiés : "<<amrSidePoints.nPointIds<<endl;
  cout<<"Nb points de feuilles : "<<amrSidePoints.nLeaves<<endl;
  cout<<"Nb points de mailles speciales : "<<amrSidePoints.nSpecials<<endl;
  cout<<"Nb points de cotés : "<<amrSidePoints.nSidePoints<<endl;
  cout<<"Moyenne points de coté : "<<amrSidePoints.nSidePoints/(double)amrSidePoints.nSpecials<<endl;

  return 0;
}
