#include "PointIds.h"

#include <iostream>
using namespace std;

using namespace hct;

template <unsigned int D> inline ostream & operator << (ostream& out, const PointIds<D>& pts)
{
  pts.toStream(out);
  return out;
}

int main()
{
  PointIds<3> ptIds(0);
  ptIds.nodes[5] = -1;
  cout<<"sizeof(PointIds<3>) = "<<sizeof(ptIds)<<endl;
  cout<<"ids = "<<ptIds<<endl;
  cout<<"contains -1 : "<<ptIds.contains(-1)<<endl;
  cout<<"contains 2 : "<<ptIds.contains(2)<<endl;
  return 0;
}

