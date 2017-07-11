#ifndef __AMR_CELL_SIZE_H
#define __AMR_CELL_SIZE_H

#include "Vec.h"

namespace hct
{
	static constexpr double SQRT2 = 1.4142135623730951;
	
  /*---------------------------------------------------------
    description de la taille d'un hypercube de dimension D
    ---------------------------------------------------------*/
  template<typename T,unsigned int D> struct AmrCellSize;


  template<typename T> struct AmrCellSize<T,0> : public Vec<T,0>
  {
    inline AmrCellSize() {}
    inline AmrCellSize(Vec<T,0>) {}
    inline bool operator < ( AmrCellSize ) const { return false; }
    inline bool operator == ( AmrCellSize ) const { return true; }
  };

  template<typename T,unsigned int D>
    struct AmrCellSize : public Vec<T,D>
    {
      typedef Vec<T,D> SC;

      inline AmrCellSize() {}
      inline AmrCellSize(const Vec<T,D>& v) : Vec<T,D>(v) {}

      inline bool operator < ( const Vec<T,D>& c ) const
      {
	if( SC::val > c.SC::val*SQRT2 )
	  {
	    return ! ( AmrCellSize<T,D-1>(c) < *this );
	  }
	else if ( c.SC::val*SQRT2 > SC::val )
	  {
	    return AmrCellSize<T,D-1>(*this) < c ;
	  }
	else
	  {
	    return false;
	  }
      }

      inline bool operator == ( const Vec<T,D>& c ) const
      {
	return ( SC::val < c.SC::val*SQRT2 )
	  && ( SC::val*SQRT2 > c.SC::val )
	  && ( AmrCellSize<T,D-1>(*this) == c );
      }
    };

}; // namespace hct

#endif
/* ================================================================== */


#ifdef _TEST_AmrCellSize

#include <iostream>
using namespace std;

using namespace hct;

typedef AmrCellSize<double,3> Size3;

ostream & operator << (ostream& out, const Size3& v)
{
  v.toStream(out);
  return out;
}

int main()
{
  Vec<double,3> a(1);
  Vec<double,3> b(1.3);
  Size3 s1(a);
  Size3 s2(b);

  cout<<"sizeof(Vec<double,3>)="<<sizeof(Vec<double,3>)<<endl;
  cout<<"sizeof(AmrCellSize<double,3>)="<<sizeof(AmrCellSize<double,3>)<<endl;

  cout<<s1<<endl;
  cout<<s2<<endl;
  cout<<(s2<s1)<<endl;

  return 0;
}

#endif
