#include "Vec.h"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <array>
#include <set>
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
  Vec3d a(0.5) , b(2);
  Vec3d c(0.33);
  Vec3i d(2);
  Vec3b e({true,false,true});

  a = Vec3i(1);

  a+=b*3;

  b = Vec3d( 6.9, Vec2d(7.1) );

  // vector type promotion test int*double=>double, bool*double=>double
  cout << d << " * " << c << " = " << (d*c) << endl;
  cout << c << " * " << d << " = " << (c*d) << endl;
  cout << c << " * " << e << " = " << (c*e) << endl;

  // other tests
  cout<<"sizeof(Vec3d)="<<sizeof(Vec3d)<<endl;
  cout<<"sizeof(Vec3i)="<<sizeof(Vec3i)<<endl;
  cout<<a<<" < "<<b<<" = "<<(a<b)<<endl;
  cout<<"min( ("<<a<<") , ("<<b<<") ) = "<<a.min(b)<<endl;
  cout<<"reduce_mul("<<b<<") = "<<b.reduce_mul()<<endl;
  cout<<"reverse("<<b<<") = "<<b.reverse()<<endl;
  cout << "map<square>("<<b<<") = " << b.map( [](double x) -> double {return x*x; } ) << endl;
  cout << "-(" << a << ") = (" << (-a) << ")"<<endl;

  // bitfield tests
  for (size_t i = 0; i < 8; i++)
  {
	  auto vertex = hct::bitfield_vec<3>(i);
	  std::cout << "bitfield(" << i << ") = " << vertex << std::endl;
  }

  cout << "test ordering\n";
  std::set<Vec3d, decltype(&Vec3d::less_operator)> vecset( & Vec3d::less_operator );
  for (size_t i = 0; i < 20; i++)
  {
	  double r[3] = { ((double)rand()) / RAND_MAX , ((double)rand()) / RAND_MAX , ((double) rand()) / RAND_MAX };
	  vecset.insert( Vec3d(r) );
  }
  for (Vec3d v : vecset)
  {
	  cout << v << std::endl;
  }

  return 0;
}
