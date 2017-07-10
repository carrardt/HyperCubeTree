#include "PathBits.h"

#include <iostream>
using namespace std;

using namespace hct;

template <typename T, unsigned int D>
inline ostream & operator << (ostream& out, const Vec<T,D>& v)
{
  v.toStream(out);
  return out;
}

int main()
{
  Vec<double,3> center(0.5);
  Vec<double,3> p( 0.7 , Vec<double,2>(0.1) );
  Vec<bool,3> path;

  path = p > center;
  cout<<"Center = "<<center<<endl;
  cout<<"Point = "<<p<<endl;
  cout<<"Path = "<<path<<endl;
  cout<<"Path bits = "<<PathBits<3>::fromPath(path)<<endl;

  Vec<bool,4> _1011( true, Vec<bool,3>(false, Vec<bool,2>(true,Vec<bool,1>(true))));
  _1011.toStream(cout); cout<<" = "<< PathBits<4>::fromPath(_1011)<<endl;

  return 0;
}
