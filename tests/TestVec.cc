#include "Vec.h"
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
  Vec3d a(0.5),b(2);
  
  a = Vec3i(1);

  a+=b*3;

  b = Vec3d( 6.9, Vec2d(7.1) );

  cout<<"sizeof(Vec3d)="<<sizeof(Vec3d)<<endl;
  cout<<"sizeof(Vec3i)="<<sizeof(Vec3i)<<endl;
  cout<<a<<" < "<<b<<" = "<<(a<b)<<endl;
  cout<<"min( ("<<a<<") , ("<<b<<") ) = "<<a.min(b)<<endl;
  cout<<"reduce_mul("<<b<<") = "<<b.reduce_mul()<<endl;
  cout<<"reverse("<<b<<") = "<<b.reverse()<<endl;
  return 0;
}
