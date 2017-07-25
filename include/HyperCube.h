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
	this means that in a cube, the faces are the object where 2 out of 3 coordinates are "free" (denoted by X)
	and one coordinate has to be fixed. the vertices will be all possible coordinates :
	(0,0,0) ; (0,0,1) ; (0,1,0) ...

	From this simple rule you can count and enumerate each constituants of an hyper cube of any dimension.
	*/

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

	  // ======================= iterate over all components ==============================
	  // templated functor
	  template<typename FuncT> inline void forEachComponent(FuncT f) const
	  { 
		  f(value, Mask());
	  }
	  template<typename FuncT> inline void forEachComponent(FuncT f)
	  { 
		  f(value, Mask());
	  }

	  // =============== iterate over components sharing a specific vertex ===============
	  // templated functor
	  template<typename FuncT> inline void forEachComponentSharingVertex(NullBitField, FuncT f)
	  {
		  f(value, Mask());
	  }
	  template<typename FuncT> inline void forEachComponentSharingVertex(NullBitField, FuncT f) const
	  {
		  f(value, Mask());
	  }

	  // ========================= iterate over all values ===============================
	  template<typename FuncT> inline void forEachValue(FuncT f) const 
	  { 
		  f(value); 
	  }
	  template<typename FuncT> inline void forEachValue(FuncT f) 
	  { 
		  f(value); 
	  }

	  // ================= iterate over all vertices ==================
	  // fixed type functor
	  template<typename FuncT> inline void forEachVertexValue(FuncT f) const
	  { 
		 f(Mask::BITFIELD, value); 
	  }
	  template<typename FuncT> inline void forEachVertexValue(FuncT f)
	  { 
		  f(Mask::BITFIELD, value);
	  }

	  // ================= iterate over all vertices ===================
	  // templated functor
	  template<typename FuncT> inline void forEachVertexComponent(FuncT f) const
	  {
		  f(value, Mask());
	  }
	  template<typename FuncT> inline void forEachVertexComponent(FuncT f)
	  {
		  f(value, Mask() );
	  }


	  inline T& operator [] (NullBitField) { return value; }
	  inline const T& operator [] (NullBitField) const { return value; }
  };

  template <typename _T, unsigned int _D, typename _Mask> struct HyperCube
  {
	  using T = _T;
	  using Mask = _Mask;
	  static constexpr unsigned int D = _D;
	  static constexpr size_t NumberOfVertices = static_cast<size_t>(1) << D;

	  // WARNING : this is tricky !
	  HyperCube< T, D - 1, CBitField<Bit0, Mask> > _0;
	  HyperCube< T, D - 1, CBitField<BitX, Mask> > _X;
	  HyperCube< T, D - 1, CBitField<Bit1, Mask> > _1;

	  inline HyperCube() : _0(), _X(), _1() {}
	  inline HyperCube(const T& defVal) : _0(defVal), _X(defVal), _1(defVal) {}
	  inline HyperCube(const HyperCube& cube) : _0(cube._0), _X(cube._X), _1(cube._1) {}

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

	  // =============== iterate over components sharing a specific vertex ===============
	  // FIXME: LE IF N'EMPECHE L'INSTANTIATION !!!!!
	  template<typename VertBF, typename FuncT>
	  inline void forEachComponentSharingVertex(VertBF, FuncT f)
	  {
		  if (VertBF::Bit::ZERO) { _0.forEachComponentSharingVertex(typename VertBF::Tail(), f); }
		  _X.forEachComponentSharingVertex(typename VertBF::Tail(), f);
		  if (VertBF::Bit::ONE) { _1.forEachComponentSharingVertex(typename VertBF::Tail(), f); }
	  }
	  template<typename VertBF, typename FuncT>
	  inline void forEachComponentSharingVertex(VertBF, FuncT f) const
	  {
		  if (VertBF::Bit::ZERO) { _0.forEachComponentSharingVertex(typename VertBF::Tail(), f); }
		  _X.forEachComponentSharingVertex(typename VertBF::Tail(), f);
		  if (VertBF::Bit::ONE) { _1.forEachComponentSharingVertex(typename VertBF::Tail(), f); }
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

	  // =============== iterate over all the vertices (0-D components) ============
	  // non templated functor
	  template<typename FuncT>
	  inline void forEachVertexValue(FuncT f)
	  {
		  _0.forEachVertexValue(f);
		  _1.forEachVertexValue(f);
	  }
	  template<typename FuncT>
	  inline void forEachVertexValue(FuncT f) const
	  {
		  _0.forEachVertexValue(f);
		  _1.forEachVertexValue(f);
	  }

	  // =============== iterate over all the vertices (0-D components) ============
	  // templated functor
	  template<typename FuncT> inline void forEachVertexComponent(FuncT f) 
	  {
		  _0.forEachVertexComponent(f);
		  _1.forEachVertexComponent(f);
	  }
	  template<typename FuncT> inline void forEachVertexComponent(FuncT f) const
	  {
		  _0.forEachVertexComponent(f);
		  _1.forEachVertexComponent(f);
	  }
 

	  // address sub element with a bitfield
	  template<typename Tail> inline T& operator [] (CBitField<Bit0,Tail>) { return _0[Tail()]; }
	  template<typename Tail> inline const T& operator [] (CBitField<Bit0, Tail>) const { return _0[Tail()]; }
	  template<typename Tail> inline T& operator [] (CBitField<BitX, Tail>) { return _X[Tail()]; }
	  template<typename Tail> inline const T& operator [] (CBitField<BitX, Tail>) const { return _X[Tail()]; }
	  template<typename Tail> inline T& operator [] (CBitField<Bit1, Tail>) { return _1[Tail()]; }
	  template<typename Tail> inline const T& operator [] (CBitField<Bit1, Tail>) const { return _1[Tail()]; }
  };


  // ==========================================================
  // ==================== Helper templates ====================
  // ==========================================================

  /* Purpose of NeighborVertex :
   * if CompBF is a CBitField describing a sub-element of the n-cube of interset,
   * to which a neighbor is connected, abd VertBF is one of the vertices of CompBF,
   * then, thresulting 'Vertex' type defined in NeighborVertex is the vertex description
   * in the neighbor n-cube.
   * 
   * Exemple : ____ ____
   *          | n  *  c |      c i cell of interest
   *          |____*____|      n is neighbor cell, attached to c through left edge (X0)
   * 
   * CompBF = X0, VertBF = 10 => NeighborVertex::Vertex = 11
   */
  template<typename CompBF, typename VertBF> struct NeighborVertex
  {
  };

  template<> struct NeighborVertex<NullBitField, NullBitField>
  {
	  using Vertex = NullBitField;
  };
  template<typename CompBFTail, typename VertBFTail>
  struct NeighborVertex< CBitField<BitX, CompBFTail>, CBitField<Bit0, VertBFTail> >
  {
	  using Vertex = CBitField<Bit0, typename NeighborVertex<CompBFTail, VertBFTail>::Vertex >;
  };
  template<typename CompBFTail, typename VertBFTail>
  struct NeighborVertex< CBitField<BitX, CompBFTail>, CBitField<Bit1, VertBFTail> >
  {
	  using Vertex = CBitField<Bit1, typename NeighborVertex<CompBFTail, VertBFTail>::Vertex >;
  };
  template<typename CompBFTail, typename VertBFTail>
  struct NeighborVertex< CBitField<Bit0, CompBFTail>, CBitField<Bit0, VertBFTail> >
  {
	  using Vertex = CBitField<Bit1, typename NeighborVertex<CompBFTail, VertBFTail>::Vertex >;
  };
  template<typename CompBFTail, typename VertBFTail>
  struct NeighborVertex< CBitField<Bit1, CompBFTail>, CBitField<Bit1, VertBFTail> >
  {
	  using Vertex = CBitField<Bit0, typename NeighborVertex<CompBFTail, VertBFTail>::Vertex >;
  };


  /* helper template to determine what is the vertex index (aka bitfield) of cell in the dual mesh,
  relative to a vertex in the primary mesh.
  */
  /*
  template<typename CompBF, typename VertBF> struct NeighborDualVertex {};
  template<> struct NeighborDualVertex<NullBitField, NullBitField>
  {
  using Vertex = NullBitField;
  };
  template<typename CompBFTail, typename VertBFTail>
  struct NeighborDualVertex< CBitField<Bit0, CompBFTail>, CBitField<Bit0, VertBFTail> >
  {
  using Vertex = CBitField<Bit0, NeighborDualVertex<CompBFTail, VertBFTail>::Vertex >;
  };
  template<typename CompBFTail, typename VertBFTail>
  struct NeighborDualVertex< CBitField<Bit1, CompBFTail>, CBitField<Bit1, VertBFTail> >
  {
  using Vertex = CBitField<Bit1, NeighborDualVertex<CompBFTail, VertBFTail>::Vertex >;
  };
  template<typename CompBFTail, typename VertBFTail>
  struct NeighborDualVertex< CBitField<BitX, CompBFTail>, CBitField<Bit0, VertBFTail> >
  {
  using Vertex = CBitField<Bit1, NeighborDualVertex<CompBFTail, VertBFTail>::Vertex >;
  };
  template<typename CompBFTail, typename VertBFTail>
  struct NeighborDualVertex< CBitField<BitX, CompBFTail>, CBitField<Bit1, VertBFTail> >
  {
  using Vertex = CBitField<Bit0, NeighborDualVertex<CompBFTail, VertBFTail>::Vertex >;
  };
  */


}; // namespace hct
