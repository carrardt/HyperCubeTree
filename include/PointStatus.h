#ifndef __POINT_STATUS_H
#define __POINT_STATUS_H

namespace hct
{

  template<typename T, unsigned int _D> struct Vec;

  template<unsigned int _D> struct PointStatus;

  template<> struct PointStatus<0>
  {
    template<typename Point> static inline bool isPointFacing(Vec<bool,0>,Vec<bool,0> ds,Point) { return true; }
    template<typename Mask, typename Point> static inline bool isSidePoint(Vec<bool,0>,Vec<bool,0>,Mask,Point) { return true; }
  };

  template<unsigned int _D> struct PointStatus
  {
    enum { D = _D };
    // retourne vrai si le point considéré est en face du point correspondant du voisin
    template<typename Point> static inline bool isPointFacing(Vec<bool,D> us, Vec<bool,D> ds, Point )
    {
      bool stick = ( Point::Bit::ZERO && !us.val ) || ( Point::Bit::ONE  && !ds.val );
      return stick && PointStatus<D-1>::isPointFacing(us,ds, typename Point::Tail() );
    }

    template<typename Mask, typename Point> static inline bool isSidePoint(Vec<bool,D> us, Vec<bool,D> ds, Mask, Point )
    {
      bool stick = ( Point::Bit::ZERO && !us.val ) || ( Point::Bit::ONE && !ds.val );
      bool slide = ( Point::Bit::ZERO && us.val ) || ( Point::Bit::ONE && ds.val ) ;
      return
	( ( (!Mask::Bit::UNDEF) && stick ) || ( Mask::Bit::UNDEF && slide ) )
	&& PointStatus<D-1>::isSidePoint(us,ds, typename Mask::Tail(), typename Point::Tail() );
    }
  };

}; // hct

#endif
/* ===================================================================================================================*/
