#pragma once

#include "Vec.h"
#include "ConstBits.h"

namespace hct
{
	/*
	CubeEnum enables enumeration of all constituants of an hypercube, that is,
	for an N-Cube : all the (N-1)-Cubes it contains, all the (N-2)-Cubes, ... , all the 0-Cubes (vertices).
	for a 3-cube (just a 3D cube) it gives :
	6 faces (2-Cubes)
	12 edges (1-Cubes)
	8 vertices (0-Cubes)

	Constituants of an Hyper Cube are denoted by succession of ternary digits : 0, 1 and X.
	For instance, the faces (2-cubes) of a 3-cube are :
	(X,X,0) ; (X,X,1) ; (X,0,X) ; (X,1,X) ; (0,X,X) ; (1,X,X).
	this means that in a cube, the face are the object where 2 out of 3 coordinates are "free" (denoted by X)
	and one coordinate has to be fixed. the vertices will be all possible coordinates :
	(0,0,0) ; (0,0,1) ; (0,1,0) ...

	From this simple rule you can count and enumerate each constituants of an hyper cube of any dimension.
	*/

  //============================== New version =============================
  /*
  One can now specialize what type is stored depending of the constituant dimensionality.
  This has to be done ...
  */

  template <typename T, unsigned int D, typename Mask = NullBitField> struct HyperCube;
  template <typename _T, typename _Mask> struct HyperCube<_T, 0, _Mask>
  {
	  using T = _T;
	  static constexpr unsigned int D = 0;
	  using Mask = typename _Mask::Reverse;

	  // Valeur associée a l'élement
	  T value;

	  inline HyperCube() {}
	  inline HyperCube(const T& defVal) : value(defVal) {}
	  inline HyperCube(const HyperCube& cube) : value(cube.value) {}

	  inline HyperCube& operator = (const HyperCube& cube)
	  {
		  value = cube.value;
	  }

	  inline T& self() { return value; }
	  inline const T& self() const { return value; }

	  template<typename FuncT> inline void forEachComponent(FuncT f) const { f(*this); }
	  template<typename FuncT> inline void forEachComponent(FuncT f) { f(*this); }

	  template<typename FuncT> inline void forEachValue(FuncT f) const { f(value); }
	  template<typename FuncT> inline void forEachValue(FuncT f) { f(value); }
  };

  template <typename _T, unsigned int _D, typename _Mask> struct HyperCube
  {
	  using T = _T;
	  using Mask = _Mask;
	  static constexpr unsigned int D = _D;

	  HyperCube< T, D - 1, CBitField<Bit0, Mask> > _0; // Ensemble des elements dont le premier bit est contraint à 0
	  HyperCube< T, D - 1, CBitField<BitX, Mask> > _X; // Ensemble des elements dont le premier bit est libre
	  HyperCube< T, D - 1, CBitField<Bit1, Mask> > _1; // Ensemble des elements dont le premier bit est contraint à 1

	  inline HyperCube() : _0(), _X(), _1() {}
	  inline HyperCube(const T& defVal) : _0(defVal), _X(defVal), _1(defVal) {}
	  inline HyperCube(const HyperCube& cube) : _0(cube._0), _X(cube._X), _1(cube._1) {}

	  inline HyperCube& operator = (const HyperCube& cube)
	  {
		  _0 = cube._0;
		  _X = cube._X;
		  _1 = cube._1;
	  }

	  inline T& self() { return _X.self(); }
	  inline const T& self() const { return _X.self(); }

	  // =============== iterate over all the sub n-cube components ===============
	  template<typename FuncT>
	  inline void forEachComponent(FuncT f)
	  {
		  _0.forEachComponent(f);
		  _X.forEachComponent(f);
		  _1.forEachComponent(f);
	  }

	  template<typename FuncT>
	  inline void forEachComponent(FuncT f) const
	  {
		  _0.forEachComponent(f);
		  _X.forEachComponent(f);
		  _1.forEachComponent(f);
	  }

	  // =============== iterate over all the values stored for each component ============
	  template<typename FuncT>
	  inline void forEachValue(FuncT f)
	  {
		  _0.forEachValue(f);
		  _X.forEachValue(f);
		  _1.forEachValue(f);
	  }

	  template<typename FuncT>
	  inline void forEachValue(FuncT f) const
	  {
		  _0.forEachValue(f);
		  _X.forEachValue(f);
		  _1.forEachValue(f);
	  }

  };



}; // namespace hct
