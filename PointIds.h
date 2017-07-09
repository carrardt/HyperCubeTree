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
