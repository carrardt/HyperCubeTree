#ifndef __POINT_IDS_H
#define __POINT_IDS_H

namespace AmrReconstruction3
{

  template <unsigned int D> struct PointIds
  {
    enum { Size = 1<<D };
    inline PointIds() {}
    inline PointIds(int n)
    {
      for(int i=0;i<Size;i++) nodes[i]=n;
    }
    inline bool contains(int n)
    {
      for(int i=0;i<Size;i++) if(nodes[i]==n) return true;
      return false;
    }

    inline const int& operator [] (int i) const { return nodes[i]; }
    inline int& operator [] (int i) { return nodes[i]; }

    template <typename StreamT>
    inline void toStream(StreamT& out) const
    {
      out<<nodes[0];
      for(unsigned int i=1;i<Size;i++)
	{
	  out<<','<<nodes[i];
	}
    }
    int nodes[Size];
  };
  
}; // namespace AmrReconstruction3

#endif
/* ===================================================================================================================*/


#ifdef _TEST_PointIds

#include <iostream>
using namespace std;

using namespace AmrReconstruction3;

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

#endif
