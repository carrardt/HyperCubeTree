#ifndef __AMR_CONNECT_H
#define __AMR_CONNECT_H

#include "AmrLevels.h"
#include "AmrTree.h"
#include "CubeEnum.h"
#include "Nbh.h"
#include "Vec.h"
#include "GridEnum.h"
#include "GridDimension.h"

namespace Amr2Ugrid
{

	using namespace hct;

  template<unsigned int _D> struct AmrConnect
  {
    enum { D = _D };

    using SlideFlags = hct::Vec<bool,D> ;
    using Coord = hct::Vec<unsigned int,D>;

    struct ElementInfo
    {
      int level;
      int node;
      SlideFlags upSliding; // decollement de la borne inferieur : > 0
      SlideFlags downSliding; // decollement de la borne superieur : < N-1
      inline ElementInfo() : level(0), node(-1), upSliding(false), downSliding(false) {}
    };

    typedef CubeEnum<ElementInfo,D> Cube;

    struct CubeInitialize
    {
      template<typename Mask> inline void processComponent( CubeEnum<ElementInfo,0,Mask>& c)
      {
	c.value = ElementInfo();
      }
      int _x;
    };

    inline AmrConnect( const AmrTree& t, const LevelInfo<D>* l )
      : tree(t), levelInfo(l)
    {
      CubeInitialize cubeInitialize;

      cubes = new Cube*[ tree.nLevels ];
      for(int i=0;i<tree.nLevels;i++)
	{
	  int ncubes = tree.nodeLevels[i].size;
	  cubes[i] = new Cube[ ncubes ];
	  for(int j=0;j<ncubes;j++)
	    {
	      cubes[i][j].forEachComponent( cubeInitialize );
	      cubes[i][j].self().level = i;
	      cubes[i][j].self().node = j;
	    }
	}
    }

    inline ~AmrConnect()
    {
      for(int i=0;i<tree.nLevels;i++)
	{
	  delete [] cubes[i];
	}      
      delete [] cubes;
    }

    // Operateur de connexion entre une extremite d'un cube et l'extremite correspondante d'un cube voisin
    template <unsigned int __D>
    struct PopulateCube
    {
      enum { D = __D };
      inline PopulateCube(AmrConnect& s) : self(s) {}

      // "parent" est le cube parent dont le fils a la coordonee "coord" est le voisin "child",
      // du noeud enfant destination
      template<typename M1, typename M2>
	inline void operator () (const CubeEnum<ElementInfo,0,M1>& parent, CubeEnum<ElementInfo,0,M2>& child, const Coord& grid, const Coord& selfCoord, Coord nbhCoord)
      {
	if( parent.value.node!=-1 )
	  {
	    // je regarde si le noeud a creuser est une feuille
	    bool isLeaf = self.tree.isLeaf(parent.value.level, parent.value.node);
	    if( isLeaf ) // je ne peut pas "creuser" dans le noeud parent
	      {
		child.value.node = parent.value.node; // je garde la trace du parent le long duquel je glisse
		child.value.level = parent.value.level; // donc le niveau reste le meme
		// le glissement s'accumule si l'on creuse plusieurs fois un noeud feuille
		// ici, le + a valeur de OU, car on travaille sur des booléens
		child.value.upSliding = parent.value.upSliding + ( selfCoord > Coord(0) ) ;
		child.value.downSliding = parent.value.downSliding + ( selfCoord < (grid-1) ) ;
	      }
	    else // je recupere le fils du noeud parent a creuser a la coordonée indiquée
	      {
		int branch = hct::GridDimension<D>(grid).branch(nbhCoord); 
		int snode = self.tree.subNode(parent.value.level, parent.value.node, branch);
		child.value.level = parent.value.level +1;
		child.value.node = snode; //snode peut etre vide (-1): le parent n'est pas une feuille mais a un fils nul.
	      }
	  }
	else // noeud parent nul
	  {
	    child.value.node = parent.value.node; // quand je creuse, je recupere un noeud nul ...
	    child.value.level = parent.value.level; // ... de meme niveau
	  }
      }

      AmrConnect& self;
    };
    
    
    // Operateur d'iteration dans la grille d'un niveau de l'arbre
    template <unsigned int __D>
    struct ForEachPieceOfGrid
    {
      enum { D = __D };
      inline ForEachPieceOfGrid(AmrConnect& s,int l,int n) : self(s), level(l), node(n) {}
      inline void operator () (const Coord& coord)
      {
	int branch = self.levelInfo[level].grid.branch( coord );
	int subNode = self.tree.subNode(level,node,branch);
	if(subNode != -1)
	  {
	    PopulateCube<D> connector(self);
	    Nbh<ElementInfo,D>::dig(
			self.levelInfo[level].grid, // taille de la grille du niveau courant
			connector, // operateur de connection d'un parent vers le voisin d'un noeud inferieur
			self.cubes[level][node], // parent
			self.cubes[level+1][subNode], // fils
			coord // coordonee a laquelle on "creuse" vers le nieveau inferieur
		     );
	    self.connectTree( level+1, subNode );
	  }
	
      }
      AmrConnect& self;
      int level, node;
    };

    inline void connectTree( int level=0, int nodeId=0 )
    {
      ForEachPieceOfGrid<D> gridParser(*this,level,nodeId);
      gridEnum( levelInfo[level].grid , gridParser  );
    }


    struct CubeStats
    {
      inline CubeStats(int& _nslides) : nslides(_nslides) {}
      template<typename T, typename Mask> inline void processComponent(const CubeEnum<T,0,Mask>& c)
      {
	if( c.value.node!=-1 && (c.value.upSliding+c.value.downSliding).reduce_add() )
	  {
	    nslides++;
	  }
      }
      int& nslides;
    };

    template <typename StreamT> inline 
    void toStream(StreamT & out) const
    {
      for(int i=0;i<tree.nLevels;i++)
	{
	  int slide=0;
	  int leaves=0;
	  for(int j=0;j<tree.nodeLevels[i].size;j++)
	    {
	      if( tree.isLeaf(i,j) )
		{
		  leaves++;
		  int s=0;
		  CubeStats stat(s);
		  cubes[i][j].forEachComponent( stat );
		  if(s!=0) slide++;
		}
	    }
	  out<<"Level "<<i<<" : slides="<<slide<<"/"<<leaves<<"\n";
	}
    }

    template <typename StreamT> inline 
    void toBinaryStream(StreamT & out) const
    {
      out.write( (char*)&(tree.nLevels) , sizeof(int) );
      for(int i=0;i<tree.nLevels;i++)
	{
	  out.write( (char*)&(tree.nodeLevels[i].size) , sizeof(int) );
	}
      for(int i=0;i<tree.nLevels;i++)
	{
	  out.write( (char*)(cubes[i]) , tree.nodeLevels[i].size*sizeof(Cube) );
	}
    }

    template <typename StreamT> inline 
    void fromBinaryStream(StreamT & in)
    {
      int nLevels = 0;
      in.read( (char*)&nLevels , sizeof(int) );
      assert( nLevels == tree.nLevels );
      cubes = new Cube*[tree.nLevels];

      int totalSize = 0;
      for(int i=0;i<tree.nLevels;i++)
	{
	  int size = 0;
	  in.read( (char*)&size , sizeof(int) );
	  assert( size == tree.nodeLevels[i].size );
	  totalSize += size;
	}
      //Cube* ptr = new Cube[totalSize];
      for(int i=0;i<tree.nLevels;i++)
	{
	  //cubes[i] = ptr;
	  cubes[i] = new Cube[ tree.nodeLevels[i].size ];
	  in.read( (char*)(cubes[i]) , tree.nodeLevels[i].size*sizeof(Cube) );
	  //ptr += tree.nodeLevels[i].size;
	}
    }

    // ---- Data ----
    const AmrTree& tree;
    const LevelInfo<D>* levelInfo;
    Cube** cubes;
  };


}; // Amr2Ugrid

#endif
