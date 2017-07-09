#ifndef __AMR_TREE_H
#define __AMR_TREE_H

#include "Vec.h"
#include "AmrLevels.h"

#include <assert.h>
#include <string.h>

namespace AmrReconstruction3
{

  struct TreeNode
  {
    int index;
    int nCells;
  };
  
  struct NodeLevel
  {
    TreeNode* nodes;
    int size;
    int capacity;
  };

  struct AmrTree
  {
    inline AmrTree() : nLevels(0), nodeLevels(0), allNodes(0) {}
    inline ~AmrTree()
    {
      if( allNodes!=0 )
	{
	  delete[] allNodes;
	}
      else
	{
	  for(int i=0;i<nLevels;i++)
	    {
	      if( nodeLevels[i].nodes!=0 ) delete [] nodeLevels[i].nodes;
	    }
	}
      if( nodeLevels!=0 ) delete[] nodeLevels;
    }

    template<unsigned int D>
    inline void initLevels(int nl, LevelInfo<D>* levels)
    {
      nLevels = nl;
      nodeLevels = new NodeLevel[nLevels];

      nodeLevels[0].capacity = 1;
      if( nLevels >= 1 ) nodeLevels[1].capacity = levels[0].grid.gridSize();
      if( nLevels >= 2 ) nodeLevels[nLevels-1].capacity = levels[nLevels-1].nCells;

      for(int i=2;i<(nLevels-1);i++)
	{
	  nodeLevels[i].capacity = levels[i-1].nCells * levels[i-1].grid.gridSize();
	}

      for(int i=0;i<nLevels;i++)
	{
	  nodeLevels[i].size = 0;
	  nodeLevels[i].nodes = new TreeNode[ nodeLevels[i].capacity ];	    
	}

      // on creer l'unique noeud racine
      nodeLevels[0].size = 1;
      nodeLevels[0].nodes[0].index = -1;
      nodeLevels[0].nodes[0].nCells = 0;
    }

    template<unsigned int D>
    inline int allocateNodes(LevelInfo<D>* levels, int depth)
    {
      int index = nodeLevels[depth+1].size;
      int nnodes = levels[depth].grid.gridSize();
      while( (nodeLevels[depth+1].size+nnodes) > nodeLevels[depth+1].capacity )
	{
	  TreeNode* tmp = new TreeNode[nodeLevels[depth+1].capacity*2];
	  memcpy( tmp, nodeLevels[depth+1].nodes, nodeLevels[depth+1].size * sizeof(TreeNode) );
	  delete [] nodeLevels[depth+1].nodes;
	  nodeLevels[depth+1].nodes = tmp;
	  nodeLevels[depth+1].capacity *= 2;
	}
      // increment du nombre de noeuds
      nodeLevels[depth+1].size += nnodes;
      // initialisation des noeuds enfants
      for(int i=0;i<nnodes;i++)
	{
	  nodeLevels[depth+1].nodes[index+i].index = -1;
	  nodeLevels[depth+1].nodes[index+i].nCells = 0;
	}
      return index;
    }

    inline int subNode(int depth, int n, int branch) const
    {
#ifdef DEBUG
      assert( depth>=0 && depth<nLevels && n>=0 && n<nodeLevels[depth].size );
#endif
      if( nodeLevels[depth].nodes[n].index==-1 )
	{
	  return -1;
	}
      else
	{
	  return nodeLevels[depth].nodes[n].index + branch;
	}
    }

    inline bool isLeaf(int depth, int n) const
    {
#ifdef DEBUG
      assert( depth>=0 && depth<nLevels && n>=0 && n<nodeLevels[depth].size );
#endif
      return nodeLevels[depth].nodes[n].index == -1 ;
    }

    template <typename T, unsigned int D>
      inline int insertCell( Vec<T,D> cellCenter, int cellDepth,
			     Vec<T,D> origin,
			     LevelInfo<D>* levelInfo, AmrCellSize<T,D>* levelSize )
    {
      int n = 0;
      int depth = 0;
      for(; depth<cellDepth; depth++)
	{
	  AmrCellSize<T,D> cellSize = levelSize[depth+1];
	  Grid<D> grid = levelInfo[depth].grid;
	  Vec<unsigned int,D> cellPos = ( cellCenter - origin ) / cellSize ;
	  unsigned int branch = grid.branch( cellPos );
	  origin += ( cellSize * cellPos );
	  if( nodeLevels[depth].nodes[n].index==-1 )
	    {
	      nodeLevels[depth].nodes[n].index = allocateNodes( levelInfo, depth );
	    }
	  n = nodeLevels[depth].nodes[n].index + branch;
	}
      nodeLevels[depth].nodes[n].nCells++;
      return n;
    }

    template <typename StreamT> inline 
    void toStream(StreamT & out) const
    {
      for(int i=0;i<nLevels;i++)
	{
	  int leaves=0;
	  int uniques=0;
	  for(int j=0;j<nodeLevels[i].size;j++)
	    {
	      if( isLeaf(i,j) )
		{
		  leaves++;
		  if( nodeLevels[i].nodes[j].nCells == 1 )
		    {
		      uniques++;
		    }
		}
	    }
	  out<<i<<" : size="<<nodeLevels[i].size<<", capacity="<<nodeLevels[i].capacity<<", "
	     <<leaves<<" leaves, "<<uniques<<" singles\n";
	}
    }

    template <typename StreamT> inline 
    void toBinaryStream(StreamT & out) const
    {
      out.write( (char*)&nLevels , sizeof(int) );
      for(int i=0;i<nLevels;i++)
	{
	  out.write( (char*)&(nodeLevels[i].size) , sizeof(int) );
	}
      for(int i=0;i<nLevels;i++)
	{
	  out.write( (char*)nodeLevels[i].nodes , nodeLevels[i].size*sizeof(TreeNode) );
	}
    }

    template <typename StreamT> inline 
    void fromBinaryStream(StreamT & in)
    {
      in.read( (char*)&nLevels , sizeof(int) );
      nodeLevels = new NodeLevel[nLevels];

      int totalSize = 0;
      for(int i=0;i<nLevels;i++)
	{
	  nodeLevels[i].capacity = 0; // empeche tout ajout
	  in.read( (char*)&(nodeLevels[i].size) , sizeof(int) );
	  totalSize += nodeLevels[i].size;
	}

      TreeNode* ptr = allNodes = new TreeNode[totalSize];
      for(int i=0;i<nLevels;i++)
	{
	  nodeLevels[i].nodes = ptr;
	  in.read( (char*)nodeLevels[i].nodes , nodeLevels[i].size*sizeof(TreeNode) );
	  ptr += nodeLevels[i].size;
	}
    }

    int nLevels;
    NodeLevel* nodeLevels;
    TreeNode* allNodes;
  };

}; // namespace AmrReconstruction3

#endif





#ifdef _TEST_AmrTree
#include <iostream>
#include <fstream>
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
#endif
