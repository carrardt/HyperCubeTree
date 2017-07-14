#pragma once

#include "Vec.h"

namespace hct
{
	template<typename _T, unsigned int _D>
	struct CSGSphere
	{
		static constexpr unsigned int D = _D;
		using T = _T;
		using VecT = hct::Vec<T, D>;

		inline CSGSphere(std::initializer_list<T> center, double radius = 1.0)
			: m_center(center.begin())
		{
			if (radius > 0.0)
			{
				m_scale = 1.0 / radius;
				m_offset = -radius;
			}
			else if (radius < 0.0)
			{
				m_scale = 1.0 / (-radius);
				m_offset = radius;
			}
		}

		inline T operator () (VecT p)
		{
			VecT v = p - m_center;
			return std::sqrt(v.dot(v)) * m_scale + m_offset;
		}
		VecT m_center;
		double m_scale = 1.0;
		double m_offset = 0.0;
	};

	//size()
	template<typename T>
	static inline auto csg_sphere(std::initializer_list<T> center,double radius=1.0)
	{
		constexpr unsigned int D = center.size();
		return CSGSphere<double,D>(center, radius);
	}

	template<typename Func1T, typename Func2T>
	struct CSGIntersection
	{
		using VecT = typename Func1T::VecT;
		using ScalarT = typename VecT::T;

		inline CSGIntersection(Func1T f1, Func2T f2)
			: m_f1(f1)
			, m_f2(f2)
		{}

		inline ScalarT operator () (VecT p)
		{
			return std::min(m_f1(p), m_f2(p));
		}

		Func1T m_f1;
		Func2T m_f2;
	};

	template<typename Func1T, typename Func2T>
	static inline
		CSGIntersection<Func1T, Func2T>
		csg_intersection(Func1T f1, Func2T f2)
	{
		return CSGIntersection<Func1T, Func2T>(f1, f2);
	}

	template<typename FuncT>
	struct CSGInside
	{
		using VecT = typename FuncT::VecT;
		inline CSGInside(FuncT f) : m_f(f) {}
		inline bool operator () (VecT p)
		{
			return m_f(p) <= 0;
		}
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