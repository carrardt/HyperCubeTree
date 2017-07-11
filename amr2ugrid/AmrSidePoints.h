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
Amr2Ugrid::MeshConnectivity<3> mesh;
Amr2Ugrid::MeshGeometry<float,3> geom;
Amr2Ugrid::AmrLevels<float,3> levels;
int* cellNode = 0;
int* cellDepth = 0;
hct::Vec<float,3>* cellCenter = 0;
hct::AmrCellSize<float,3>* cellSize = 0;
int** nodeCellId = 0;
std::set<int> ** sidePointSet = 0;
#endif

#ifdef PROFILING
#include <sys/time.h>
#endif

#include "PointConnect.h"
#include "InterLevelPointConnect.h"
#include "SidePointInsert.h"

namespace Amr2Ugrid
{

	using namespace hct;

  template<unsigned int _D>
  struct AmrSidePoints
  {
    enum { D = _D };
    using PointIds = hct::PointIds<D>;
    using LevelInfo = Amr2Ugrid::LevelInfo<D>;
    using AmrConnect = Amr2Ugrid::AmrConnect<D>;

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

}; // Amr2Ugrid

#endif
