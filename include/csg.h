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

	/*
	CSG surface functions return a plane.
	this is the tangent plane of the nearest surface plane, expressed as if origin was point p, passed in parameter.
	in other words, it returns :
	make_vec( signed_distance_to_the_surface , surface_normal )
	*/

	static constexpr double SurfEpsilon = 1.e-16;

	// ================= generic surface function interface ==================
	template<unsigned int _D>
	struct ICSGSurface
	{
		static constexpr unsigned int D = _D;
		using VecT = hct::Vec<double, _D>;
		using PlaneT = hct::Vec<double, _D+1>;
		virtual PlaneT operator () (VecT p) const = 0;
	};

	template<unsigned int _D>
	struct CSGSurfaceDelegate 
	{
		static constexpr unsigned int D = _D;
		using VecT = hct::Vec<double,D>;
		using PlaneT = hct::Vec<double, _D + 1>;
		inline CSGSurfaceDelegate(ICSGSurface<D>* surf) : m_surf(surf) {}
		inline PlaneT operator () (VecT p) const
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
		using PlaneT = hct::Vec<double, D + 1>;
		inline CSGSphere(Vec<double,D> center, double radius) : m_center(center) , m_radius(radius) {}
		inline CSGSphere(std::initializer_list<double> center, double radius) : CSGSphere(Vec<double, D>(center), radius) {}
		// evaluation returns the tangent plane on the surface, nearest to p
		inline PlaneT operator () (VecT p) const override final
		{
			VecT normal = p - m_center;
			double dist = std::sqrt(normal.dot(normal));
			if (dist > SurfEpsilon) { normal /= dist; }
			else { normal = VecT(0.0); }
			return make_vec( dist - m_radius , normal) ;
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
		inline PlaneT operator () (VecT p) const override final
		{
			double dist = make_vec(1.0,p).dot(m_plane) ;
			Vec<double, D> normal(m_plane);
			return make_vec(dist, normal);
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
		using PlaneT = typename FuncT::PlaneT;
		inline CSGNegate(FuncT f) : m_f(f) {}
		inline PlaneT operator () (VecT p) const override final
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
		using PlaneT = typename Func1T::PlaneT;
		inline CSGUnion(Func1T f1, Func2T f2) : m_f1(f1) , m_f2(f2) {}
		inline PlaneT operator () (VecT p)	const override final
		{
			PlaneT plane1 = m_f1(p);
			PlaneT plane2 = m_f2(p);
			double d1 = plane1.val;
			double d2 = plane2.val;
			VecT n1(plane1);
			VecT n2(plane2);
			VecT normal;
			if (d1 <= d2) { normal = n1; }
			else { normal = n2; }
			return make_vec( std::min(d1,d2), normal);
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
		using PlaneT = typename Func1T::PlaneT;
		inline CSGIntersection(Func1T f1, Func2T f2) : m_f1(f1)	, m_f2(f2) {}
		inline PlaneT operator () (VecT p) const override final
		{ 
			PlaneT plane1 = m_f1(p);
			PlaneT plane2 = m_f2(p);
			double d1 = plane1.val;
			double d2 = plane2.val;
			VecT n1(plane1);
			VecT n2(plane2);
			VecT normal;
			if (d1 >= d2) { normal = n1; }
			else { normal = n2; }
			return make_vec(std::max(d1, d2), normal);
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
		inline bool operator () (VecT p) const { return m_f(p).val <= 0; }
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
