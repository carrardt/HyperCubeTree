#pragma once

#include "Vec.h"

#include <cmath>
#include <algorithm>
#include <memory>

namespace hct
{
	/*
		CSG here is based on implicit surfaces.
		Surface functions return a scalar value for any point in space.
		Wherever this function returns 0 indicate a point on the surface.
		a value <0 indicates a point "behind" the surface, or "inside" if the surface is closed.
		a fvalue >0 indicates a point "beyond" or "outside".
	*/

	// ================= generic surface function interface ==================
	template<unsigned int _D>
	struct ICSGSurface
	{
		static constexpr unsigned int D = _D;
		using VecT = hct::Vec<double, _D>;
		virtual double operator () (VecT p) const = 0;
	};

	template<unsigned int _D>
	struct CSGSurfaceDelegate 
	{
		static constexpr unsigned int D = _D;
		using VecT = hct::Vec<double,D>;
		inline CSGSurfaceDelegate(ICSGSurface<D>* surf) : m_surf(surf) {}
		inline double operator () (VecT p) const
		{
			return m_surf->operator () (p);
		}
		ICSGSurface<D>* m_surf;
	};

	// ================= sphere ==================
	template<unsigned int _D>
	struct CSGSphere : public ICSGSurface<_D>
	{
		static constexpr unsigned int D = _D;
		using VecT = hct::Vec<double, D>;
		inline CSGSphere(Vec<double,D> center, double radius) : m_center(center) , m_radius(radius) {}
		inline CSGSphere(std::initializer_list<double> center, double radius) : CSGSphere(Vec<double, D>(center), radius) {}
		inline double operator () (VecT p) const override final
		{
			VecT v = p - m_center;
			return std::sqrt(v.dot(v)) - m_radius;
		}

		VecT m_center;
		double m_radius = 0.0;
	};

	template<unsigned int D> static inline CSGSphere<D>
	csg_sphere(Vec<double,D> center,double radius) { return CSGSphere<D>(center, radius); }

	template<unsigned int D> static inline CSGSphere<D>*
	csg_sphere_new(Vec<double, D> center, double radius) { return new CSGSphere<D>(center, radius); }

	// ================= plane ==================
	template<unsigned int _D>
	struct CSGPlane : public ICSGSurface<_D>
	{
		static constexpr unsigned int D = _D;
		using VecT = hct::Vec<double, D>;
		using PlaneT = hct::Vec<double, D+1>;
		inline CSGPlane(Vec<double, D> normal, double offset) : m_plane(offset,normal) {}
		inline CSGPlane(std::initializer_list<double> coeficients) : m_plane(coeficients) {}
		inline double operator () (VecT p) const override final
		{
			return p.dot(m_plane) + m_plane.val;
		}

		PlaneT m_plane;
	};

	template<unsigned int D> static inline CSGPlane<D>
	csg_plane(Vec<double, D> normal, double offset) { return CSGPlane<D>(normal, offset); }

	template<unsigned int D> static inline CSGPlane<D>*
	csg_plane_new(Vec<double, D> normal, double offset) { return new CSGPlane<D>(normal, offset); }

	// ================ Inverse operator ==================
	template<typename FuncT>
	struct CSGNegate : public ICSGSurface<FuncT::D>
	{
		static constexpr unsigned int D = FuncT::D;
		using VecT = typename FuncT::VecT;
		using ScalarT = typename VecT::T;
		inline CSGNegate(FuncT f) : m_f(f) {}
		inline ScalarT operator () (VecT p) const override final
		{
			return - m_f(p) ;
		}

		FuncT m_f;
	};

	template<typename FuncT> static inline CSGNegate<FuncT>
	csg_negate(FuncT f) { return CSGNegate<FuncT>(f); }

	template<typename FuncT> static inline CSGNegate<FuncT>*
	csg_negate_new(FuncT f) { return new CSGNegate<FuncT>(f); }

	// ================ Union operator ==================
	template<typename Func1T, typename Func2T>
	struct CSGUnion : public ICSGSurface<Func1T::D>
	{
		static constexpr unsigned int D = Func1T::D;
		using VecT = typename Func1T::VecT;
		using ScalarT = typename VecT::T;
		inline CSGUnion(Func1T f1, Func2T f2) : m_f1(f1) , m_f2(f2) {}
		inline ScalarT operator () (VecT p)	const override final
		{ 
			return std::min(m_f1(p), m_f2(p)); 
		}
		Func1T m_f1;
		Func2T m_f2;
	};

	template<typename Func1T, typename Func2T> static inline CSGUnion<Func1T, Func2T> 
	csg_union(Func1T f1, Func2T f2) { return CSGUnion<Func1T, Func2T>(f1, f2); }

	template<typename Func1T, typename Func2T> static inline CSGUnion<Func1T, Func2T>*
	csg_union_new(Func1T f1, Func2T f2) { return new CSGUnion<Func1T, Func2T>(f1, f2); }

	// ================ Intersection operator ==================
	template<typename Func1T, typename Func2T>
	struct CSGIntersection : public ICSGSurface<Func1T::D>
	{
		static constexpr unsigned int D = Func1T::D;
		using VecT = typename Func1T::VecT;
		using ScalarT = typename VecT::T;
		inline CSGIntersection(Func1T f1, Func2T f2) : m_f1(f1)	, m_f2(f2) {}
		inline ScalarT operator () (VecT p) const override final
		{ 
			return std::max(m_f1(p), m_f2(p)); 
		}
		Func1T m_f1;
		Func2T m_f2;
	};

	template<typename Func1T, typename Func2T> static inline CSGIntersection<Func1T, Func2T>
	csg_intersection(Func1T f1, Func2T f2) { return CSGIntersection<Func1T, Func2T>(f1, f2); }

	template<typename Func1T, typename Func2T> static inline CSGIntersection<Func1T, Func2T>*
	csg_intersection_new(Func1T f1, Func2T f2) { return new CSGIntersection<Func1T, Func2T>(f1, f2); }

	template<typename Func1T, typename Func2T> static inline CSGIntersection<Func1T, CSGNegate<Func2T> >
	csg_difference(Func1T f1, Func2T f2) { return CSGIntersection<Func1T, CSGNegate<Func2T> >(f1, csg_negate(f2) ); }

	template<typename Func1T, typename Func2T> static inline CSGIntersection<Func1T, CSGNegate<Func2T> >*
	csg_difference_new(Func1T f1, Func2T f2) { return new CSGIntersection<Func1T, CSGNegate<Func2T> >(f1, csg_negate(f2)); }


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
