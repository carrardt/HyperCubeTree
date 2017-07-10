#ifndef __AMR_VEC_H
#define __AMR_VEC_H

#include <math.h>

namespace hct
{

  template <typename T,unsigned int D>
    struct Vec;

  template <typename T>
    struct Vec<T,0>
    {
      enum { Size = 0 };

      inline Vec() {}
      inline Vec(T) {}
      template<typename T2> inline Vec(Vec<T2,0>) {}

      template<typename T2> inline void fromArray(const T2*) const {}

      template<typename StreamT> inline void toStream(StreamT& out) const {}
      inline Vec operator = (Vec) const {}

      inline Vec reverse() const { return Vec(); }

      inline T reduce_mul() const { return (T)1; }
      inline T reduce_add() const { return (T)0; }
#define REDUCE_BOOL(func) inline bool reduce_##func() const { return false; }
      REDUCE_BOOL(isnan);
      REDUCE_BOOL(isinf);
      REDUCE_BOOL(isfinite);
      REDUCE_BOOL(isnormal);
#undef REDUCE_BOOL

      template<typename T2> inline T gridIndex(Vec<T2,0>) const { return (T)0; }

      template<typename T2> inline Vec min(Vec<T2,0>) const { return Vec(); }
      template<typename T2> inline Vec max(Vec<T2,0>) const { return Vec(); }

      template<typename T2> inline T dot(Vec<T2,0>) const { return (T)0; }
      inline T length2() const { return (T)0; }

#define BINARY_VEC_OPERATOR(OP) template<typename T2> inline Vec operator OP (const Vec<T2,0> op) const { return Vec(); }
#define BOOL_VEC_OPERATOR(OP) template<typename T2> inline Vec<bool,0> operator OP (const Vec<T2,0>& op) const { return Vec<bool,0>(); }
#define SELF_VEC_OPERATOR(OP)   template<typename T2> inline Vec operator OP (const Vec<T2,0> op) const { return Vec(); }
#define BINARY_SCAL_OPERATOR(OP) inline Vec operator OP (const T& op) const { return Vec(); }
#define SELF_SCAL_OPERATOR(OP)   inline Vec operator OP (const T& op) const { return Vec(); }

      BINARY_VEC_OPERATOR(+);
      BINARY_VEC_OPERATOR(-);
      BINARY_VEC_OPERATOR(*);
      BINARY_VEC_OPERATOR(/);

      BINARY_SCAL_OPERATOR(+);
      BINARY_SCAL_OPERATOR(-);
      BINARY_SCAL_OPERATOR(*);
      BINARY_SCAL_OPERATOR(/);

      SELF_VEC_OPERATOR(+=);
      SELF_VEC_OPERATOR(-=);
      SELF_VEC_OPERATOR(*=);
      SELF_VEC_OPERATOR(/=);

      SELF_SCAL_OPERATOR(+=);
      SELF_SCAL_OPERATOR(-=);
      SELF_SCAL_OPERATOR(*=);
      SELF_SCAL_OPERATOR(/=);

      BOOL_VEC_OPERATOR(<);
      BOOL_VEC_OPERATOR(>);
      BOOL_VEC_OPERATOR(<=);
      BOOL_VEC_OPERATOR(>=);
      BOOL_VEC_OPERATOR(==);
      BOOL_VEC_OPERATOR(!=);

#undef BOOL_VEC_OPERATOR
#undef BINARY_VEC_OPERATOR
#undef BINARY_SCAL_OPERATOR
#undef SELF_VEC_OPERATOR
#undef SELF_SCAL_OPERATOR
      //T __useless;
    };



  template<typename T,unsigned int D,unsigned int Size> struct Reverse;
  template<typename T,unsigned int D> struct Reverse<T,D,0>
  {
    static inline Vec<T,0> reverse(const Vec<T,D>&) { return Vec<T,0>(); }
  };
  template<typename T,unsigned int D,unsigned int Size> struct Reverse
  {
    static inline Vec<T,Size> reverse(const Vec<T,D>& v)
    {
      return Vec<T,Size>( v.Vec<T,D-Size+1>::val, Reverse<T,D,Size-1>::reverse(v) );
    }
  };
  
  template < typename T, unsigned int D >
    struct Vec : public Vec<T,D-1>
    {
      enum { Size = D };
 
      T val;

      // constructeur par defaut
      inline Vec() : Vec<T,D-1>() {}

      // constructeur par copie
      template<typename T2> inline Vec(const Vec<T2,D>& vec ) : val((T)vec.val), Vec<T,D-1>(vec) {}

      // constructeur par initialisation de toutes les composantes a une meme valeur
      inline Vec(const T& ival) : Vec<T,D-1>(ival), val(ival) {}

      // constructeur par ajout d'une valeur en tete d'un veteur de dimension D-1
      inline Vec(const T& head, const Vec<T,D-1>& v ) : val(head), Vec<T,D-1>(v) {}

      // constructeur a partir d'un tableau d'éléments
      template <typename T2> inline Vec(const T2* array) { this->fromArray(array); }
      template <typename T2> inline void fromArray(const T2* coord)
      {
	val=(T)coord[D-1]; this->Vec<T,D-1>::fromArray(coord);
      }

      // ecriture du vecteur dans un flot texte
      template<typename StreamT> inline void toStream(StreamT& out) const
      {
	out<<val;
	if(D>1) {
	  out<<',';
	  Vec<T,D-1>::toStream(out);
	}
      }

      inline Vec& operator = (const Vec& src)
      {
	val = (T) src.val;
	Vec<T,D-1>::operator = ( src );
	return *this;
      }
      
      // inversion x,y,z -> z,y,x
      inline Vec reverse() const
      {
	return Reverse<T,D,D>::reverse( *this );
      }

      // fonctions de reduction
      inline T reduce_mul() const { return val * Vec<T,D-1>::reduce_mul(); }
      inline T reduce_add() const { return val + Vec<T,D-1>::reduce_add(); }
#define REDUCE_BOOL(func) inline bool reduce_##func() const { return func(val) ||  Vec<T,D-1>::reduce_##func(); }
      REDUCE_BOOL(isnan);
      REDUCE_BOOL(isinf);
      REDUCE_BOOL(isfinite);
      REDUCE_BOOL(isnormal);
#undef REDUCE_BOOL

      // fonctions pour gerer les index dans les grilles regulieres
      template<typename T2> inline T gridIndex(const Vec<T2,D>& pos) const { return pos.val * Vec<T,D-1>::reduce_mul() + Vec<T,D-1>::gridIndex(pos) ; }

      // operations algebriques
      template<typename T2> inline T dot(const Vec<T2,D>& op) const { return (T)( (val*op.val) + Vec<T,D-1>::dot(op) ); }
      inline T length2() const { return (T)( (val*val) + Vec<T,D-1>::length2() ); }

      // min/max composante a composante
      template<typename T2> inline Vec min(const Vec<T2,D>& op) const { return (val<=op.val) ? Vec(val,Vec<T,D-1>::min(op)) : Vec(op.val,Vec<T,D-1>::min(op)) ; }
      template<typename T2> inline Vec max(const Vec<T2,D>& op) const { return (val>=op.val) ? Vec(val,Vec<T,D-1>::max(op)) : Vec(op.val,Vec<T,D-1>::max(op)) ; }

#define BINARY_VEC_OPERATOR(OP) template<typename T2> inline Vec operator OP (const Vec<T2,D>& op) const { return Vec( (T)(val OP op.val), Vec<T,D-1>::operator OP (op) ); }
#define BOOL_VEC_OPERATOR(OP) template<typename T2> inline Vec<bool,D> operator OP (const Vec<T2,D>& op) const { return Vec<bool,D>( (bool)(val OP op.val) , Vec<T,D-1>::operator OP (op) ); }
#define SELF_VEC_OPERATOR(OP) template<typename T2> inline Vec& operator OP (const Vec<T2,D>& op) { val OP ((T) op.val) ; Vec<T,D-1>::operator OP (op); return *this; }
#define BINARY_SCAL_OPERATOR(OP) inline Vec  operator OP (const T& op) const { return Vec( val OP op , Vec<T,D-1>::operator OP (op) ); }
#define SELF_SCAL_OPERATOR(OP) inline Vec& operator OP (const T& op) { val OP op; Vec<T,D-1>::operator OP (op); return *this; }

      BINARY_VEC_OPERATOR(+);
      BINARY_VEC_OPERATOR(-);
      BINARY_VEC_OPERATOR(*);
      BINARY_VEC_OPERATOR(/);

      BINARY_SCAL_OPERATOR(+);
      BINARY_SCAL_OPERATOR(-);
      BINARY_SCAL_OPERATOR(*);
      BINARY_SCAL_OPERATOR(/);

      SELF_VEC_OPERATOR(+=);
      SELF_VEC_OPERATOR(-=);
      SELF_VEC_OPERATOR(*=);
      SELF_VEC_OPERATOR(/=);

      SELF_SCAL_OPERATOR(+=);
      SELF_SCAL_OPERATOR(-=);
      SELF_SCAL_OPERATOR(*=);
      SELF_SCAL_OPERATOR(/=);

      BOOL_VEC_OPERATOR(<);
      BOOL_VEC_OPERATOR(>);
      BOOL_VEC_OPERATOR(<=);
      BOOL_VEC_OPERATOR(>=);
      BOOL_VEC_OPERATOR(==);
      BOOL_VEC_OPERATOR(!=);

#undef BOOL_VEC_OPERATOR
#undef BINARY_VEC_OPERATOR
#undef BINARY_SCAL_OPERATOR
#undef SELF_VEC_OPERATOR
#undef SELF_SCAL_OPERATOR
    };

  typedef Vec<double,2> Vec2d;
  typedef Vec<double,3> Vec3d;
  typedef Vec<double,4> Vec4d;

  typedef Vec<float,2> Vec2f;
  typedef Vec<float,3> Vec3f;
  typedef Vec<float,4> Vec4f;

  typedef Vec<int,2> Vec2i;
  typedef Vec<int,3> Vec3i;
  typedef Vec<int,4> Vec4i;

  typedef Vec<unsigned int,2> Vec2ui;
  typedef Vec<unsigned int,3> Vec3ui;
  typedef Vec<unsigned int,4> Vec4ui;

}; // namespace hct

#endif //__AMR_VEC_H
