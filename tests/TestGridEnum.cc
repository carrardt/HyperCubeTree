#include "GridEnum.h"

#include <iostream>
using namespace std;
using namespace hct;

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
