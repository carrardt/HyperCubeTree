#pragma once

#include "Vec.h"
#include "ConstBits.h"

namespace hct
{

// Enumeration de l'ensemble des constituants d'un D-cube (i.e. tous les n-cubes internes, pour 0<=n<=D)
template <typename T, unsigned int D, typename Mask = NullBitField> struct CubeEnum;
template <typename _T, typename _Mask> struct CubeEnum<_T, 0, _Mask>
{
	using T = _T;
	enum { D = 0 };

	// !! par construction, l'empilement de bits d�finissant l'�l�ment est � l'envers, d'o� le "::Reverse" !!
	using Mask = typename _Mask::Reverse;

	// Valeur associ�e a l'�lement
	T value;

	inline CubeEnum() {}
	inline CubeEnum(const T& defVal) : value(defVal) {}
	inline CubeEnum(const CubeEnum& cube) : value(cube.value) {}

	inline CubeEnum& operator = (const CubeEnum& cube)
	{
		value = cube.value;
	}

	inline T& self() { return value; }
	inline const T& self() const { return value; }

	template<typename ComponentProcessor>
	inline void forEachComponent(ComponentProcessor& proc)
	{
		proc.processComponent(*this);
	}
};

template <typename _T, unsigned int _D, typename _Mask> struct CubeEnum
{
	using T = _T;
	using Mask = _Mask;
	static constexpr unsigned int D = _D;

	CubeEnum< T, D - 1, CBitField<Bit0, Mask> > _0; // Ensemble des elements dont le premier bit est contraint � 0
	CubeEnum< T, D - 1, CBitField<BitX, Mask> > _X; // Ensemble des elements dont le premier bit est libre
	CubeEnum< T, D - 1, CBitField<Bit1, Mask> > _1; // Ensemble des elements dont le premier bit est contraint � 1

	inline CubeEnum() : _0(), _X(), _1() {}
	inline CubeEnum(const T& defVal) : _0(defVal), _X(defVal), _1(defVal) {}
	inline CubeEnum(const CubeEnum& cube) : _0(cube._0), _X(cube._X), _1(cube._1) {}

	inline CubeEnum& operator = (const CubeEnum& cube)
	{
		_0 = cube._0;
		_X = cube._X;
		_1 = cube._1;
	}

	inline T& self() { return _X.self(); }
	inline const T& self() const { return _X.self(); }

	template<typename ComponentProcessor>
	inline void forEachComponent(ComponentProcessor& proc)
	{
		_0.forEachComponent(proc);
		_X.forEachComponent(proc);
		_1.forEachComponent(proc);
	}

	template<typename ComponentProcessor>
	inline void forEachComponent(ComponentProcessor& proc) const
	{
		_0.forEachComponent(proc);
		_X.forEachComponent(proc);
		_1.forEachComponent(proc);
	}
};


}