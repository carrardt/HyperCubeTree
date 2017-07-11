#ifndef __AMR_TREE_H
#define __AMR_TREE_H

#include "Vec.h"
#include "AmrLevels.h"

#include <assert.h>
#include <string.h>

namespace Amr2Ugrid
{

	using namespace hct;

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

	/*
	 * Make room for children of one node a specified depth.
	 * return index of first children, relative to to the specified level storage.
	 */
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
#		ifdef DEBUG
			assert( depth>=0 && depth<nLevels && n>=0 && n<nodeLevels[depth].size );
#		endif
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
#		ifdef DEBUG
		assert( depth>=0 && depth<nLevels && n>=0 && n<nodeLevels[depth].size );
#		endif
		return nodeLevels[depth].nodes[n].index == -1 ;
	}

	/*
	 * Insert a new cell, given a position and the depth in the tree.
	 * This specifically tailored to transform and unstructured mesh to a tree.
	 */
	template <typename T, unsigned int D>
	inline int insertCell(
		Vec<T,D> cellCenter,
		int cellDepth,
		Vec<T,D> origin,
		LevelInfo<D>* levelInfo,
		AmrCellSize<T,D>* levelSize )
	{
		int n = 0;
		int depth = 0;
		for(; depth<cellDepth; depth++)
		{
			AmrCellSize<T,D> cellSize = levelSize[depth+1];
			GridDimension<D> grid = levelInfo[depth].grid;
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

	/*
	 * Write tree structure to file
	 */
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

	/*
	 * Read tree structure from file
	 */
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

}; // namespace hct

#endif
