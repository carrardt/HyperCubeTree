#include "GridEnum.h"

#include <iostream>
#include <vector>
#include <assert.h>

using namespace hct;

char tmp[256];
template<unsigned int D> struct Input;
template<> struct Input<0>
{
  static inline void getUIVec(Vec<unsigned int,0>)
  {
    std::cin.getline(tmp,255,'\n');
  }
};
template<unsigned int D> struct Input
{
  static inline void getUIVec( Vec<unsigned int,D>& v )
  {
    std::cin>>v.val;
    Input<D-1>::getUIVec(v);
  }
};

struct TestFunctor
{
  inline TestFunctor(std::ostream& o)
	  : out(o) {}

  inline void operator () (const Vec<unsigned int,3>& coord)
  {
	  m_posvector.push_back(coord);
    out<<'('; coord.toStream(out);
    out<<")\n";
  }

  std::vector<Vec3ui> m_posvector;
  std::ostream& out;
};

int main()
{
  GridDimension<3> grid;
  std::cout<<"Grid (3 values) ? "; std::cout.flush();
  Input<3>::getUIVec(grid);
  std::cout<<"Enumeration of "; grid.toStream(std::cout); std::cout<< std::endl;
  
  TestFunctor testfunc(std::cout);
  gridEnum( grid, testfunc);

  // this is needed for CellVertexConnectivity to work properly
  for (size_t i = 1; i < testfunc.m_posvector.size(); i++)
  {
	  assert( testfunc.m_posvector[i - 1].less(testfunc.m_posvector[i]) );
  }

  std::cout<< "test ok" << std::endl;

  return 0;
}
