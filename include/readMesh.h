#ifndef __READ_MESH_H
#define __READ_MESH_H

#include "Vec.h"
#include "PointIds.h"

#include <fstream>
#include <string>

namespace AmrReconstruction3
{

  static inline bool readMesh(const char* fileName,
			      int& nc, int& np,
			      PointIds<3>*& indices,
			      Vec3f*& points,
			      float*& scalars)
  {
    std::ifstream fic(fileName);
    if( !fic.good() ) return false;

    // lecture du nombre de mailles et nombre de points
    fic.read( (char*)&nc, sizeof(int) ); // nombre de mailles
    fic.read( (char*)&np, sizeof(int) ); // nombre de points

    indices = new PointIds<3>[nc];
    points = new Vec3f[np];
    scalars = new float[nc];

    // lecture du maillage
    for(int i=0;i<nc;i++)
      {
	int ncp;
	fic.read( (char*)&ncp , sizeof(int) );
	if(ncp==PointIds<3>::Size) // on verifie que c'est un tetra
	  {
	    fic.read( (char*)(indices+i) , sizeof(PointIds<3>) );
	  }
	else // on saute la maille foireuse
	  {
	    for(int j=0;j<8;j++) indices[i].nodes[j] = -1;
	    fic.ignore(ncp*sizeof(int));
	  }
      }
    fic.read( (char*)points , np*sizeof(Vec3f) );
    fic.read( (char*)scalars , nc*sizeof(float) );

    fic.close();
    return true;
  }

}; // namespace AmrReconstruction3

#endif //__AMR_VEC_H

