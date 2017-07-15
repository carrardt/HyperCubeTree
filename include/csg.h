#pragma once

#include "Vec.h"

#include <cmath>
#include <algorithm>

namespace hct
{


	// ================= sphere ==================
	template<unsigned int _D>
	struct CSGSphere
	{
		static constexpr unsigned int D = _D;
		using VecT = hct::Vec<double, D>;
		inline CSGSphere(Vec<double,D> center, double radius) : m_center(center) , m_radius(radius) {}
		inline CSGSphere(std::initializer_list<double> center, double radius) : CSGSphere(Vec<double, D>(center), radius) {}
		inline double operator () (VecT p) const
		{
			VecT v = p - m_center;
			return std::sqrt(v.dot(v)) - m_radius;
		}
		VecT m_center;
		double m_radius = 0.0;
	};

	template<unsigned int D>
	static inline CSGSphere<D>
	csg_sphere(Vec<double,D> center,double radius) { return CSGSphere<D>(center, radius); }


	// ================ Inverse operator ==================
	template<typename FuncT>
	struct CSGNegate
	{
		using VecT = typename FuncT::VecT;
		using ScalarT = typename VecT::T;
		inline CSGNegate(FuncT f) : m_f(f) {}
		inline ScalarT operator () (VecT p) const { return - m_f(p) ; }
		FuncT m_f;
	};

	template<typename FuncT>
	static inline CSGNegate<FuncT>
	csg_negate(FuncT f) { return CSGNegate<FuncT>(f); }

	// ================ Union operator ==================
	template<typename Func1T, typename Func2T>
	struct CSGUnion
	{
		using VecT = typename Func1T::VecT;
		using ScalarT = typename VecT::T;
		inline CSGUnion(Func1T f1, Func2T f2) : m_f1(f1) , m_f2(f2) {}
		inline ScalarT operator () (VecT p)	const { return std::min(m_f1(p), m_f2(p)); }
		Func1T m_f1;
		Func2T m_f2;
	};

	template<typename Func1T, typename Func2T>
	static inline CSGUnion<Func1T, Func2T> 
	csg_union(Func1T f1, Func2T f2) { return CSGUnion<Func1T, Func2T>(f1, f2); }

	// ================ Intersection operator ==================
	template<typename Func1T, typename Func2T>
	struct CSGIntersection
	{
		using VecT = typename Func1T::VecT;
		using ScalarT = typename VecT::T;
		inline CSGIntersection(Func1T f1, Func2T f2) : m_f1(f1)	, m_f2(f2) {}
		inline ScalarT operator () (VecT p) const { return std::max(m_f1(p), m_f2(p)); }
		Func1T m_f1;
		Func2T m_f2;
	};

	template<typename Func1T, typename Func2T>
	static inline CSGIntersection<Func1T, Func2T>
	csg_intersection(Func1T f1, Func2T f2) { return CSGIntersection<Func1T, Func2T>(f1, f2); }

	template<typename Func1T, typename Func2T>
	static inline CSGIntersection<Func1T, CSGNegate<Func2T> >
	csg_difference(Func1T f1, Func2T f2) { return CSGIntersection<Func1T, CSGNegate<Func2T> >(f1, csg_negate(f2) ); }

	// ============= boolean inside operator =============
	// true means inside, false means outside
	template<typename FuncT>
	struct CSGInside
	{
		using VecT = typename FuncT::VecT;
		inline CSGInside(FuncT f) : m_f(f) {}
		inline bool operator () (VecT p) const { return m_f(p) <= 0; }
		FuncT m_f;
	};

	template<typename FuncT>
	static inline
	CSGInside<FuncT>
	csg_inside(FuncT f)
	{
		return CSGInside<FuncT>(f);
	}

}
