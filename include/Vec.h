#pragma once

#include <initializer_list>
#include <cstdint>
#include <cstddef>
#include <cmath>

namespace hct
{

	// a simple helper template to deal with boolean operations that are non-sense (*,/,-)
	template<typename T> struct vec_operation_helper
	{
		static inline auto add(const T& a, const T& b) ->decltype(a+b) { return a + b; }
		static inline auto sub(const T& a, const T& b) ->decltype(a - b) { return a - b; }
		static inline auto mul(const T& a, const T& b) ->decltype(a * b) { return a * b; }
		static inline auto div(const T& a, const T& b) ->decltype(a / b) { return a / b; }
	};
	template<> struct vec_operation_helper<bool>
	{
		static inline bool add(const bool a, const bool b) { return ( a || b ); }
		static inline bool sub(const bool a, const bool b) { return ( a ^ b ); }
		static inline bool mul(const bool a, const bool b) { return ( a && b ); }
		static inline bool div(const bool a, const bool b) { return ( a && b ); }
	};


	/*
	The Vec template class is meant to represent vectors of fixed size and type.
	They are recursively defined to fit the needs of hyper-cube neighborhood finding algorithm, mainly.
	*/

	template <typename T, unsigned int D>
	struct Vec;

	template <typename T>
	struct Vec<T, 0>
	{
		enum { Size = 0 };

		inline Vec() {}
		template<typename T2> inline Vec(T2) {}
		template<typename T2> inline Vec(Vec<T2, 0>) {}
		template<typename T2> inline Vec(const T2*) {}

		template<typename T2> inline void fromArray(const T2*) const {}
		static inline Vec<T, 0> fromBitfield(size_t) { return Vec<T, 0>(); }

		template<typename StreamT> inline void toStream(StreamT& out) const {}
		inline Vec operator = (Vec) const { return Vec(); }

		inline Vec reverse() const { return Vec(); }

		template<typename FuncT>
		inline void apply(FuncT f) {}
		template<typename FuncT>
		inline Vec map(FuncT f) const { return Vec(); }

		// return neutral value for each operator
		inline T reduce_mul() const { return static_cast<T>(1); }
		inline T reduce_add() const { return static_cast<T>(0); }
		inline bool reduce_and() const { return true; }
		inline bool reduce_or() const { return false; }
		inline T reduce_max() const { return std::numeric_limits<T>::lowest(); }

		template<typename T2> inline Vec min(Vec<T2, 0>) const { return Vec(); }
		template<typename T2> inline Vec max(Vec<T2, 0>) const { return Vec(); }
		inline Vec abs() const { return Vec(); }

		template<typename T2> inline T dot(Vec<T2, 0>) const { return (T)0; }
		inline T length2() const { return (T)0; }

		inline bool less(Vec) const { return false; }

#define BINARY_VEC_OPERATOR(OP) template<typename T2> inline Vec<decltype(T() OP T2()),0> operator OP (const Vec<T2,0> b) const { return Vec<decltype(T() OP T2()),0>(); }
#define BOOL_VEC_OPERATOR(OP) template<typename T2> inline Vec<bool,0> operator OP (const Vec<T2,0>& b) const { return Vec<bool,0>(); }
#define SELF_VEC_OPERATOR(OP)   template<typename T2> inline Vec operator OP (const Vec<T2,0> b) const { return Vec(); }
#define BINARY_SCAL_OPERATOR(OP) template<typename T2> inline Vec<decltype(T() OP T2()),0> operator OP (const T2& b) const { return Vec<decltype(T() OP T2()),0>(); }
#define SELF_SCAL_OPERATOR(OP)   inline Vec operator OP (const T& b) const { return Vec(); }

		BINARY_VEC_OPERATOR(+);
		BINARY_VEC_OPERATOR(-);
		BINARY_VEC_OPERATOR(*);
		BINARY_VEC_OPERATOR(/ );

		BINARY_SCAL_OPERATOR(+);
		BINARY_SCAL_OPERATOR(-);
		BINARY_SCAL_OPERATOR(*);
		BINARY_SCAL_OPERATOR(/ );

		SELF_VEC_OPERATOR(+= );
		SELF_VEC_OPERATOR(-= );
		SELF_VEC_OPERATOR(*= );
		SELF_VEC_OPERATOR(/= );

		SELF_SCAL_OPERATOR(+= );
		SELF_SCAL_OPERATOR(-= );
		SELF_SCAL_OPERATOR(*= );
		SELF_SCAL_OPERATOR(/= );

		BOOL_VEC_OPERATOR(< );
		BOOL_VEC_OPERATOR(> );
		BOOL_VEC_OPERATOR(<= );
		BOOL_VEC_OPERATOR(>= );
		BOOL_VEC_OPERATOR(== );
		BOOL_VEC_OPERATOR(!= );

#undef BOOL_VEC_OPERATOR
#undef BINARY_VEC_OPERATOR
#undef BINARY_SCAL_OPERATOR
#undef SELF_VEC_OPERATOR
#undef SELF_SCAL_OPERATOR
		//T __useless;
	};


	// Reverse alogrithm : Transform Vec( x1, Vec(x2 , Vec(x3) ) ) to Vec( x3, Vec(x2 , Vec(x1) ) )
	template<typename T, unsigned int D, unsigned int Size> struct Reverse;
	template<typename T, unsigned int D> struct Reverse<T, D, 0>
	{
		static inline Vec<T, 0> reverse(const Vec<T, D>&) { return Vec<T, 0>(); }
	};
	template<typename T, unsigned int D, unsigned int Size> struct Reverse
	{
		static inline Vec<T, Size> reverse(const Vec<T, D>& v)
		{
			return Vec<T, Size>(v.Vec<T, D - Size + 1>::val, Reverse<T, D, Size - 1>::reverse(v));
		}
	};

	// General case, whith D>0
	template < typename _T, unsigned int _D >
	struct Vec : public Vec<_T, _D - 1>
	{
		static constexpr unsigned int D = _D;
		static constexpr unsigned int Size = _D ;
		using T = _T;

		T val;

		// constructeur par defaut
		inline Vec() : Vec<T, D - 1>() {}

		// constructeur par copie
		template<typename T2> inline Vec(const Vec<T2, D>& vec) : val((T)vec.val), Vec<T, D - 1>(vec) {}

		// constructeur par initialisation de toutes les composantes a une meme valeur
		inline Vec(const T& ival) : Vec<T, D - 1>(ival), val(ival) {}

		// constructeur par ajout d'une valeur en tete d'un veteur de dimension D-1
		inline Vec(const T& head, const Vec<T, D - 1>& v) : val(head), Vec<T, D - 1>(v) {}

		// constructeur a partir d'un tableau d'éléments
		template <typename T2> inline Vec(const T2* array) { this->fromArray(array); }
		template <typename T2> inline Vec(std::initializer_list<T2> l) { this->fromArray(l.begin()); }

		static inline Vec<T,D> fromBitfield(size_t n)
		{
			return Vec<T,D> ( static_cast<T>( (n >> (D - 1)) & 1), Vec<T, D - 1>::fromBitfield(n));
		}

		template <typename T2> inline void fromArray(const T2* coord)
		{
			val = static_cast<T>( coord[D - 1] );
			this->Vec<T, D - 1>::fromArray(coord);
		}

		// ecriture du vecteur dans un flot texte
		template<typename StreamT> inline StreamT& toStream(StreamT& out) const
		{
			if (D > 1) {
				Vec<T, D - 1>::toStream(out);
				out << ',';
			}
			out << val;
			return out;
		}

		inline Vec& operator = (const Vec& src)
		{
			val = (T)src.val;
			Vec<T, D - 1>::operator = (src);
			return *this;
		}

		// inversion x,y,z -> z,y,x
		inline Vec reverse() const
		{
			return Reverse<T, D, D>::reverse(*this);
		}

		template<typename FuncT>
		inline void apply(FuncT f)
		{
			val = f(val);
			Vec<T, D - 1>::apply(f);
		}

		template<typename FuncT>
		inline Vec map(FuncT f) const
		{
			return Vec( f(val) , Vec<T, D - 1>::map(f) );
		}

		// FIXME: available operators (and reduction functions) should be conditioned to the type of T (bool, numeric, integer, floating point, etc.)

		// fonctions de reduction
		inline T reduce_mul() const { return vec_operation_helper<T>::mul( val , Vec<T, D - 1>::reduce_mul() ); }
		inline T reduce_add() const { return vec_operation_helper<T>::add( val , Vec<T, D - 1>::reduce_add() ); }
		inline bool reduce_and() const { return (static_cast<bool>(val) && Vec<T, D - 1>::reduce_and()); }
		inline bool reduce_or() const { return (static_cast<bool>(val) || Vec<T, D - 1>::reduce_and()); }
		inline T reduce_max() const { return std::max(val, Vec<T, D - 1>::reduce_max()); }

		// Valeur absolue
		inline Vec abs() const { return Vec( std::abs(val), Vec<T, D - 1>::abs()); }


		// operations algebriques
		template<typename T2> inline T dot(const Vec<T2, D>& op) const { return (T)((val*op.val) + Vec<T, D - 1>::dot(op)); }
		inline T length2() const { return (T)((val*val) + Vec<T, D - 1>::length2()); }

		// min/max composante a composante
		template<typename T2> inline Vec min(const Vec<T2, D>& op) const { return (val <= op.val) ? Vec(val, Vec<T, D - 1>::min(op)) : Vec(op.val, Vec<T, D - 1>::min(op)); }
		template<typename T2> inline Vec max(const Vec<T2, D>& op) const { return (val >= op.val) ? Vec(val, Vec<T, D - 1>::max(op)) : Vec(op.val, Vec<T, D - 1>::max(op)); }

		// lexicographic order (equivalent to '<' for basic types)
		inline bool less(const Vec& v) const { return val<v.val || ( val==v.val && Vec<T, D - 1>::less(v) ); }

#define BINARY_VEC_OPERATOR(OP) \
	template<typename T2> inline \
	Vec<decltype(T() OP T2()),D> \
	operator OP (const Vec<T2,D>& b) const \
	{ return Vec<decltype(T() OP T2()),D>( val OP b.val, Vec<T,D-1>::operator OP (b) ); }

#define BINARY_SCAL_OPERATOR(OP) \
	template<typename T2> inline \
	Vec<decltype(T() OP T2()),D> \
	operator OP (const T2& b) const \
    { return Vec<decltype(T() OP T2()),D>( val OP b , Vec<T,D-1>::operator OP (b) ); }

#define BOOL_VEC_OPERATOR(OP) template<typename T2> inline Vec<bool,D> operator OP (const Vec<T2,D>& rhs) const { return Vec<bool,D>( static_cast<bool>(val OP rhs.val) , Vec<T,D-1>::operator OP (rhs) ); }
#define SELF_VEC_OPERATOR(OP) template<typename T2> inline Vec& operator OP (const Vec<T2,D>& rhs) { val OP ((T) rhs.val) ; Vec<T,D-1>::operator OP (rhs); return *this; }
#define SELF_SCAL_OPERATOR(OP) inline Vec& operator OP (const T& rhs) { val OP rhs; Vec<T,D-1>::operator OP (rhs); return *this; }

		BINARY_VEC_OPERATOR(+);
		BINARY_VEC_OPERATOR(-);
		BINARY_VEC_OPERATOR(*);
		BINARY_VEC_OPERATOR(/ );

		BINARY_SCAL_OPERATOR(+);
		BINARY_SCAL_OPERATOR(-);
		BINARY_SCAL_OPERATOR(*);
		BINARY_SCAL_OPERATOR(/ );

		SELF_VEC_OPERATOR(+= );
		SELF_VEC_OPERATOR(-= );
		SELF_VEC_OPERATOR(*= );
		SELF_VEC_OPERATOR(/= );

		SELF_SCAL_OPERATOR(+= );
		SELF_SCAL_OPERATOR(-= );
		SELF_SCAL_OPERATOR(*= );
		SELF_SCAL_OPERATOR(/= );

		BOOL_VEC_OPERATOR(< );
		BOOL_VEC_OPERATOR(> );
		BOOL_VEC_OPERATOR(<= );
		BOOL_VEC_OPERATOR(>= );
		BOOL_VEC_OPERATOR(== );
		BOOL_VEC_OPERATOR(!= );

#undef BOOL_VEC_OPERATOR
#undef BINARY_VEC_OPERATOR
#undef BINARY_SCAL_OPERATOR
#undef SELF_VEC_OPERATOR
#undef SELF_SCAL_OPERATOR
	};

	typedef Vec<double, 1> Vec1d;
	typedef Vec<double, 2> Vec2d;
	typedef Vec<double, 3> Vec3d;
	typedef Vec<double, 4> Vec4d;

	typedef Vec<float, 1> Vec1f;
	typedef Vec<float, 2> Vec2f;
	typedef Vec<float, 3> Vec3f;
	typedef Vec<float, 4> Vec4f;

	typedef Vec<int, 1> Vec1i;
	typedef Vec<int, 2> Vec2i;
	typedef Vec<int, 3> Vec3i;
	typedef Vec<int, 4> Vec4i;

	typedef Vec<unsigned int, 1> Vec1ui;
	typedef Vec<unsigned int, 2> Vec2ui;
	typedef Vec<unsigned int, 3> Vec3ui;
	typedef Vec<unsigned int, 4> Vec4ui;

	typedef Vec<bool, 1> Vec1b;
	typedef Vec<bool, 2> Vec2b;
	typedef Vec<bool, 3> Vec3b;
	typedef Vec<bool, 4> Vec4b;

	template<unsigned int D>
	static inline
	Vec<int64_t, D>
	bitfield_vec(uint64_t n)
	{
		return Vec<int64_t, D>::fromBitfield(n);
	}

}; // namespace hct

