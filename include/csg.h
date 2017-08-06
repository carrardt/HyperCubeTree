#pragma once

#include <algorithm>
#include <type_traits>

#include "Vec.h"
#include "scalarfunction.h"

namespace hct
{
	/*
		CSG here is based on implicit surfaces.
		Surface functions return a scalar value for any point in space.
		Wherever this function returns 0 indicate a point on the surface.
		a value <0 indicates a point "behind" the surface, or "inside" if the surface is closed.
		a fvalue >0 indicates a point "beyond" or "outside".
	*/

	/*
	CSG surface functions return a plane.
	this is the tangent plane of the nearest surface plane, expressed as if origin was point p, passed in parameter.
	in other words, it returns :
	make_vec( signed_distance_to_the_surface , surface_normal )
	*/

	// ================ Union operator ==================
	template<typename Func1T, typename Func2T, bool is_valid = (Func1T::D == Func2T::D) && std::is_same<typename Func1T::T, typename Func2T::T>::value > struct CSGUnion {};
	template<typename Func1T, typename Func2T>
	struct CSGUnion<Func1T, Func2T,true> : public IScalarFunction<Func1T::D, typename Func1T::T>
	{
		using T = typename Func1T::T;
		static constexpr unsigned int D = Func1T::D;
		inline CSGUnion(Func1T f1, Func2T f2) : m_f1(f1), m_f2(f2) {}
		inline ScalarFunctionValue<D, T> operator () (const hct::Vec<T, D>& p) const override final
		{
			auto value1 = m_f1(p);
			auto value2 = m_f2(p);
			T d1 = value1.m_value;
			T d2 = value2.m_value;
			auto n1 = value1.m_gradient;
			auto n2 = value2.m_gradient;
			auto normal = n2;
			if (d1 <= d2) { normal = n1; }
			return ScalarFunctionValue<D, T>{ std::min(d1, d2), normal };
		}
		Func1T m_f1;
		Func2T m_f2;
	};

	template<typename Func1T, typename Func2T>
	inline CSGUnion<Func1T, Func2T> csg_union(Func1T f1, Func2T f2)
	{
		return CSGUnion<Func1T, Func2T>(f1, f2);
	}

	// ================ Intersection operator ==================
	template<typename Func1T, typename Func2T, bool is_valid = (Func1T::D == Func2T::D) && std::is_same<typename Func1T::T, typename Func2T::T>::value > struct CSGIntersection {};
	template<typename Func1T, typename Func2T>
	struct CSGIntersection<Func1T, Func2T,true> : public IScalarFunction<Func1T::D, typename Func1T::T>
	{
		using T = typename Func1T::T;
		static constexpr unsigned int D = Func1T::D;
		inline CSGIntersection(Func1T f1, Func2T f2) : m_f1(f1), m_f2(f2) {}
		inline ScalarFunctionValue<D, T> operator () (const hct::Vec<T, D>& p) const override final
		{
			auto value1 = m_f1(p);
			auto value2 = m_f2(p);
			T d1 = value1.m_value;
			T d2 = value2.m_value;
			auto n1 = value1.m_gradient;
			auto n2 = value2.m_gradient;
			auto normal = n2;
			if (d1 >= d2) { normal = n1; }
			return ScalarFunctionValue<D, T>{ std::max(d1, d2), normal };
		}
		Func1T m_f1;
		Func2T m_f2;
	};

	template<typename Func1T, typename Func2T>
	inline CSGIntersection<Func1T, Func2T> csg_intersection(Func1T f1, Func2T f2)
	{
		return CSGIntersection<Func1T, Func2T>(f1, f2);
	}

	template<typename Func1T, typename Func2T>
	inline CSGIntersection<Func1T, NegateFunction<Func2T> > csg_difference(Func1T f1, Func2T f2)
	{
		return CSGIntersection<Func1T, NegateFunction<Func2T> >( f1, NegateFunction<Func2T>(f2) );
	}

	template<unsigned int D, typename T>
	inline
	AddFunction< PointDistanceFunction<D,T> , ConstantFunction<D,T> >
	csg_sphere(const hct::Vec<T, D>& center, const T& radius)
	{
		return add_function( PointDistanceFunction<D, T>(center), ConstantFunction<D, T>(-radius) );
	}

	// ============= boolean inside operator =============
	// true means inside, false means outside
	template<typename FuncT>
	struct CSGInside
	{
		using T = typename FuncT::T;
		static constexpr unsigned int D = FuncT::D;
		inline bool operator () (const hct::Vec<T, D>& p) const { return m_f(p).m_value < 0; }
		FuncT m_f;
	};

	template<typename FuncT>
	inline CSGInside<FuncT> csg_inside(FuncT f)
	{
		return CSGInside<FuncT>{f};
	}

}
