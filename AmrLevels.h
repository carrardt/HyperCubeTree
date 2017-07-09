#ifndef __AMR_LEVELS_H
#define __AMR_LEVELS_H

#include "Vec.h"
#include "AmrCellSize.h"

#include <map>

namespace AmrReconstruction3
{

  template<unsigned int D>
    struct Grid : public Vec<unsigned int,D>
    {
      inline Grid() : Vec<unsigned int,D>(1) {}
      inline Grid(const Vec<unsigned int,D>& v) : Vec<unsigned int,D>(v) {}
      inline unsigned int gridSize() const { return this->reduce_mul(); }
      inline unsigned int branch( const Vec<unsigned int,D>& pos ) const { return this->gridIndex(pos); }
      inline Grid& operator = (const Vec<unsigned int,D>& v) { this->Vec<unsigned int,D>::operator = (v); return *this; }
    };

  template<unsigned int D>
    struct LevelInfo
    {
      Grid<D> grid;
      int depth;
      int nCells;
      inline LevelInfo() : grid(), depth(-1), nCells(0) {}
    };

  template <typename T,unsigned int D>
    struct AmrLevels
    {
		using AmrCellSize = AmrReconstruction3::AmrCellSize<T,D>;
        using Grid = AmrReconstruction3::Grid<D>;
		using LevelInfo = AmrReconstruction3::LevelInfo<D> ;

      typedef std::map<AmrCellSize,LevelInfo> LevelMap;     
      typedef typename LevelMap::iterator LevelMapIterator;
      typedef typename LevelMap::const_iterator LevelMapCIterator;
 
      inline AmrLevels() : nLevels(0), levelInfo(0), levelSize(0) {}

      inline ~AmrLevels()
      {
	if( levelInfo != 0 ) delete [] levelInfo;
	if( levelSize != 0 ) delete [] levelSize;
      }

      inline void fromMap( LevelMap& levelMap )
      {
	LevelMapIterator it = levelMap.begin();
	LevelMapIterator pit = levelMap.end();
	LevelMapIterator nit ;

	LevelMap normalizedLevelMap;
	AmrCellSize psize;

	unsigned int depth = 0;

	while(it!=levelMap.end())
	  {
	    nit = it; ++nit;

	    AmrCellSize size;
	    if( pit != levelMap.end() )
	      {
		size = psize / pit->second.grid;
	      }
	    else
	      {
		size = it->first;
	      }

	    if( nit != levelMap.end() )
	      {
		it->second.grid = ( size / nit->first ) + 0.5 ;
	      }
	    normalizedLevelMap[size] = it->second;
	    normalizedLevelMap[size].depth = depth++;

	    psize = size;
	    pit = it;
	    it = nit;
	  }

	levelMap = normalizedLevelMap;

	nLevels = levelMap.size();
	levelInfo = new LevelInfo[nLevels];
	levelSize = new AmrCellSize[nLevels];
	int i=0;
	for(LevelMapIterator it=levelMap.begin(); it!=levelMap.end(); ++it)
	  {
	    levelInfo[i] = it->second;
	    levelSize[i] = it->first;
	    i++;
	  }
      }

      template <typename StreamT> inline 
      void toStream(StreamT & out) const
      {
	for(int i=0;i<nLevels;i++)
	  {
	    out<<i<<" : (";
	    levelInfo[i].grid.toStream(out);
	    out<<")="<<levelInfo[i].grid.gridSize()<<" : (";
	    levelSize[i].toStream(out);
	    out<<") : depth="<<levelInfo[i].depth<<", nCells="<<levelInfo[i].nCells<<"\n";
	  }
      }

      template <typename StreamT> inline 
      void toBinaryStream(StreamT & out) const
      {
	out.write( (char*) &nLevels, sizeof(int) );
	out.write( (char*) levelInfo, sizeof(LevelInfo)*nLevels );
	out.write( (char*) levelSize, sizeof(AmrCellSize)*nLevels );
      }

      template <typename StreamT> inline 
      void fromBinaryStream(StreamT & in)
      {
	in.read( (char*) &nLevels, sizeof(int) );
	levelInfo = new LevelInfo[nLevels];
	levelSize = new AmrCellSize[nLevels];	
	in.read( (char*) levelInfo, sizeof(LevelInfo)*nLevels );
	in.read( (char*) levelSize, sizeof(AmrCellSize)*nLevels );
      }

      int nLevels;
      LevelInfo* levelInfo;
      AmrCellSize* levelSize;
    };

}; // namespace AmrReconstruction3

#endif
/* ============================================================ */


#ifdef _TEST_AmrLevels
#include <iostream>
#include <fstream>
using namespace std;

#include "MeshInfo.h"
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
#endif
