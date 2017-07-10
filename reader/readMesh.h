#ifndef __READ_MESH_H
#define __READ_MESH_H

#include "Vec.h"
#include "PointIds.h"

#include <fstream>
#include <string>

namespace hctreader
{

  bool readMesh(const char* fileName,
			      int& nc, int& np,
			      hct::PointIds<3>*& indices,
			      hct::Vec3f*& points,
			      float*& scalars);
}

#endif //__READ_MESH_H

