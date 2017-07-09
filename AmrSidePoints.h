#ifndef __AMR_SIDE_POINTS_H
#define __AMR_SIDE_POINTS_H

//#define DEBUG 1

#include "AmrTree.h"
#include "AmrConnect.h"
#include "AmrLevels.h"
#include "PointIds.h"
#include "PointStatus.h"


#ifdef DEBUG
#include <iostream>
#include <set>

#include "MeshInfo.h"
#include "PathBits.h"
AmrReconstruction3::MeshConnectivity<3> mesh;
AmrReconstruction3::MeshGeometry<float,3> geom;
AmrReconstruction3::AmrLevels<float,3> levels;
int* cellNode = 0;
int* cellDepth = 0;
AmrReconstruction3::Vec<float,3>* cellCenter = 0;
AmrReconstruction3::AmrCellSize<float,3>* cellSize = 0;
int** nodeCellId = 0;
std::set<int> ** sidePointSet = 0;
#endif

#ifdef PROFILING
#include <sys/time.h>
#endif

#include "PointConnect.h"
#include "InterLevelPointConnect.h"
#include "SidePointInsert.h"

namespace AmrReconstruction3
{

  template<unsigned int _D>
  struct AmrSidePoints
  {
    enum { D = _D };
    using PointIds = AmrReconstruction3::PointIds<D>;
    using LevelInfo = AmrReconstruction3::LevelInfo<D>;
    using AmrConnect = AmrReconstruction3::AmrConnect<D>;

    inline AmrSidePoints() : sidePoints(0), pointIds(0), nPointIds(0), sidePointArray(0) {}

    inline void init( const AmrTree& tree )
    {
      this->pointIds = new PointIds*[ tree.nLevels ];
      this->sidePoints = new int*[ tree.nLevels ];
      this->nPointIds = 0;
      for(int i=0;i<tree.nLevels;i++)
	{
	  int ncubes = tree.nodeLevels[i].size;
	  this->pointIds[i] = new PointIds[ ncubes ];
	  this->sidePoints[i] = new int[ ncubes ];
	  for(int j=0;j<ncubes;j++)
	    {    
	      this->sidePoints[i][j] = 0;
	      for(int k=0;k<PointIds::Size;k++)
		{
		  this->pointIds[i][j][k] = nPointIds++;
		}
	    }
	}
      this->pointIdMap = new int [nPointIds];
    }

    inline void unifyPoints( const AmrTree& tree, const LevelInfo* levelInfo, const AmrConnect& sideConnectivity)
    {
      ElementConnect<D> elementConnect(*this);

      for(int i=0;i<tree.nLevels;i++)
	{
	  elementConnect.level = i;
	  int ncubes = tree.nodeLevels[i].size;
	  for(int j=0;j<ncubes;j++)
	    {
	      //if( tree.isLeaf(i,j) )
		{
		  elementConnect.node = j;
		  sideConnectivity.cubes[i][j].forEachComponent(elementConnect);
		}
	    }

	  // on oublie pas de fusionner les points partages entre parents et enfants
	  if( i < (tree.nLevels-1) )
	    {
	      for(int j=0;j<ncubes;j++)
		{
		  if( ! tree.isLeaf(i,j) )
		    {
		      InterLevelPointConnect<D>::connect( pointIds[i][j],
							  pointIds[i+1] + tree.nodeLevels[i].nodes[j].index,
							  levelInfo[i].grid );
		    }
		}
	    }
	}
      int nid=0;
      for(int i=0;i<nPointIds;i++) pointIdMap[i] = -1;
      for(int i=0;i<tree.nLevels;i++)
	{
	  int ncubes = tree.nodeLevels[i].size;
	  for(int j=0;j<ncubes;j++)
	    {
	      for(int k=0;k<PointIds::Size;k++)
		{
		  int ptId = pointIds[i][j][k];
		  if( pointIdMap[ptId] == -1 ) pointIdMap[ptId] = nid++;
		  pointIds[i][j][k] = pointIdMap[ptId];
		}
	    }
	}
      nPointIds = nid;
    }

    inline void markPointOwners( const AmrTree& tree )
    {
      for(int i=0;i<nPointIds;i++) pointIdMap[i] = -1;
      int nid=0;
      for(int i=0;i<tree.nLevels;i++)
	{
	  int ncubes = tree.nodeLevels[i].size;
	  for(int j=0;j<ncubes;j++)
	    {
	      for(int k=0;k<PointIds::Size;k++)
		{
		  int ptId = pointIds[i][j][k];
		  if( pointIdMap[ptId] == -1 )
		    {
		      pointIdMap[ptId] = nid++;
		    }
		  else 
		    {
		      pointIds[i][j][k] = -pointIds[i][j][k] -1;
		    }
		}
	    }
	}
      nPointIds = nid;      
    }

   inline void countSidePoints( const AmrTree& tree, const AmrConnect& sideConnectivity )
    {
      CountElementSidePoints<D> countElementSidePoints(*this);
      for(int i=0;i<tree.nLevels;i++)
	{
	  countElementSidePoints.level = i;
	  int ncubes = tree.nodeLevels[i].size;
	  for(int j=0;j<ncubes;j++)
	    {
	      if( tree.isLeaf(i,j) )
		{
		  countElementSidePoints.node = j;
		  sideConnectivity.cubes[i][j].forEachComponent(countElementSidePoints);
		}
	    }
	}
      nSidePoints = 0;
      nSpecials = 0;
      nLeaves = 0;
      for(int i=0;i<tree.nLevels;i++)
	{
	  int ncubes = tree.nodeLevels[i].size;
	  for(int j=0;j<ncubes;j++)
	    {
	      if( tree.isLeaf(i,j) )
		{
		  nLeaves++;
		  int ns = sidePoints[i][j];
#ifdef DEBUG
		  if( ns != sidePointSet[i][j].size() )
		    {
		      std::cout<<"cell["<<i<<"]["<<j<<"] : "<<ns<< ((ns<sidePointSet[i][j].size())?"<":">")
				<<sidePointSet[i][j].size()<<std::endl;
		    }
#endif
		  if( ns>0 )
		    {
		      nSidePoints += ns;
		      nSpecials ++;
		    }
		}
	    }
	}
    }

    inline void restorePointIdMap( const AmrTree& tree )
    {
      int nid=0;
      for(int i=0;i<tree.nLevels;i++)
	{
	  int ncubes = tree.nodeLevels[i].size;
	  for(int j=0;j<ncubes;j++)
	    {
	      for(int k=0;k<PointIds::Size;k++)
		{
		  int ptId = pointIds[i][j][k];
		  if( ptId < 0 )
		    {
		      pointIds[i][j][k] = ( (-ptId) -1 );
		    }
		  pointIdMap[nid++] = pointIds[i][j][k];
		}
	    }
	}
    } 

   inline void buildSidePointArray( const AmrTree& tree, const AmrConnect& sideConnectivity )
    {
      this->sidePointArray = new int[ this->nSidePoints ];
      for(int i=0;i<this->nSidePoints;i++) this->sidePointArray[i]=-1;
      //this->nSidePoints = 0; // should have the same value as formerly after computation
/*
      for(int i=0;i<tree.nLevels;i++)
	{
	  int ncubes = tree.nodeLevels[i].size;
	  for(int j=0;j<ncubes;j++)
	    {
	      if( tree.isLeaf(i,j) )
		{
		  countElementSidePoints.node = j;
		  sideConnectivity.cubes[i][j].forEachComponent(countElementSidePoints);
		}
	    }
	}
*/
    }

    PointIds ** pointIds;
    int * pointIdMap;
    int nPointIds;
    int ** sidePoints;
    int nSidePoints;
    int nSpecials;
    int nLeaves;
    int * sidePointArray;
  };

}; // AmrReconstruction3

#endif
/* ===================================================================================================================*/


#ifdef _TEST_AmrSidePoints

#include <iostream>
#include <fstream>
#include <string>
using namespace std;
using namespace AmrReconstruction3;

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
   struct timeval T1;
   gettimeofday(&T1,0);
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
   struct timeval T2;
   gettimeofday(&T2,0);
   cout << "Elapsed time = " << (T2.tv_sec-T1.tv_sec)*1000.0 + (T2.tv_usec-T1.tv_usec)*0.001 << " msec"<<endl;
#endif


  cout<<"Nb points unifiés : "<<amrSidePoints.nPointIds<<endl;
  cout<<"Nb points de feuilles : "<<amrSidePoints.nLeaves<<endl;
  cout<<"Nb points de mailles speciales : "<<amrSidePoints.nSpecials<<endl;
  cout<<"Nb points de cotés : "<<amrSidePoints.nSidePoints<<endl;
  cout<<"Moyenne points de coté : "<<amrSidePoints.nSidePoints/(double)amrSidePoints.nSpecials<<endl;

  return 0;
}

#endif
