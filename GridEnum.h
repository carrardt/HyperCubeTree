#ifndef __GRID_ENUM_H
#define __GRID_ENUM_H

#include "Vec.h"
#include "AmrLevels.h"

namespace AmrReconstruction3
{
  template <unsigned int D, unsigned int D2, typename ProcObj> struct GridEnum;

  template <unsigned int D2, typename ProcObj> struct GridEnum<0,D2,ProcObj>
  {
    static inline void enumerate(Grid<0>, const Vec<unsigned int,D2> coord, ProcObj& proc)
    {
      proc( coord.reverse() );
    }    
  };

  template <unsigned int D, unsigned int D2, typename ProcObj> struct GridEnum
  {
    static inline void enumerate(const Grid<D>& grid, const Vec<unsigned int,D2> coord, ProcObj& proc)
    {
      for(unsigned int i=0;i<grid.val;i++)
	{
	  GridEnum<D-1,D2+1,ProcObj>::enumerate( grid, Vec<unsigned int,D2+1>(i,coord), proc );
	}
    }
  };

  template <unsigned int D, typename ProcObj>
    static inline void gridEnum(const Grid<D>& grid, ProcObj& proc)
    {
      GridEnum<D,0,ProcObj>::enumerate(grid,Vec<unsigned int,0>(),proc);
    }

}; // AmrReconstruction3

#endif
/* ===================================================================================================================*/


#ifdef _TEST_GridEnum

#include <iostream>
using namespace std;
using namespace AmrReconstruction3;

char tmp[256];
template<unsigned int D> struct Input;
template<> struct Input<0>
{
  static inline void getUIVec(Vec<unsigned int,0>)
  {
    cin.getline(tmp,255,'\n');
  }
};
template<unsigned int D> struct Input
{
  static inline void getUIVec( Vec<unsigned int,D>& v )
  {
    cin>>v.val;
    Input<D-1>::getUIVec(v);
  }
};

struct TestObj
{
  inline TestObj(ostream& o) : out(o) {}

  inline void operator () (const Vec<unsigned int,3>& coord)
  {
    out<<'('; coord.toStream(out);
    out<<")\n";
  }

  ostream& out;
};

int main()
{
  Grid<3> grid;
  cout<<"Grid (3 values) ? "; cout.flush();
  Input<3>::getUIVec(grid);
  cout<<"Enumeration of "; grid.toStream(cout); cout<<endl;
  
  TestObj printer(cout);
  gridEnum( grid, printer );

  cout<<endl;

  return 0;
}

#endif
