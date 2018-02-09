#pragma once

#include <cmath>
#include <memory>
#include <type_traits>
#include <algorithm>

#include "Vec.h"

namespace hct
{

	static constexpr double SurfEpsilon = 1.e-16;

	// ================= returned by a scalar function ==================
	template<unsigned int D, typename T = double>
	struct ScalarFunctionValue
	{
		inline ScalarFunctionValue(const T& value, const hct::Vec<T, D>& grad)
			: m_value(value)
			, m_gradient(grad)
		{}

		inline ScalarFunctionValue operator + (const ScalarFunctionValue& v) const
		{
			return ScalarFunctionValue(m_value + v.m_value, m_gradient + v.m_gradient);
		}

		inline ScalarFunctionValue operator - (const ScalarFunctionValue& v) const
		{
			return ScalarFunctionValue(m_value - v.m_value, m_gradient - v.m_gradient);
		}

		inline ScalarFunctionValue operator - () const
		{
			return ScalarFunctionValue(-m_value, -m_gradient);
		}

		inline T value() const { return m_value; }
		inline hct::Vec<T, D> gradient() const { return m_gradient; }

		T m_value;
		hct::Vec<T, D> m_gradient;
	};

	// ================= generic scalar function interface ==================
	template<unsigned int _D, typename _T = double>
	struct IScalarFunction
	{
		using T = _T;
		static constexpr unsigned int D = _D;
		virtual ScalarFunctionValue<D, T> operator () (const hct::Vec<T, D>& p) const = 0;
	};

	// ================= generic scalar function placeholder ==================
	template<unsigned int _D, typename _T = double>
	struct ScalarFunctionDelegate : public IScalarFunction<_D, _T>
	{
		using T = _T;
		static constexpr unsigned int D = _D;
		inline ScalarFunctionDelegate(IScalarFunction<D, T>* fptr) : m_f_ptr(fptr) {}
		inline ScalarFunctionValue<D, T> operator () (const hct::Vec<T, D>& p) const override final
		{
			return m_f_ptr->operator () (p);
		}
		std::shared_ptr< IScalarFunction<D, T> > m_f_ptr;
	};

	template<typename FuncT>
	inline
	ScalarFunctionDelegate<FuncT::D, typename FuncT::T>
	scalar_function_delegate(FuncT f)
	{
		return ScalarFunctionDelegate<FuncT::D, typename FuncT::T>( new FuncT(f) );
	}

	// ================= constant function ==================
	template<unsigned int _D, typename _T = double>
	struct ConstantFunction : public IScalarFunction<_D, _T>
	{
		using T = _T;
		static constexpr unsigned int D = _D;
		inline ConstantFunction(const T& x) : m_value(x) {}
		inline ScalarFunctionValue<D, T> operator () (const hct::Vec<T, D>& p) const override final
		{
			return ScalarFunctionValue<D, T>(m_value, hct::Vec<T, D>(0));
		}
		T m_value;
	};

	// ================= point distance function ==================
	template<unsigned int _D, typename _T = double>
	struct PointDistanceFunction : public IScalarFunction<_D, _T>
	{
		using T = _T;
		static constexpr unsigned int D = _D;
		inline PointDistanceFunction(const hct::Vec<T, D>& center) : m_center(center) {}
		// evaluation returns the tangent plane on the surface, nearest to p
		inline ScalarFunctionValue<D, T> operator () (const hct::Vec<T, D>& p) const override final
		{
			hct::Vec<T, D> normal = p - m_center;
			T dist = std::sqrt(normal.dot(normal));
			if (dist > SurfEpsilon) { normal /= dist; }
			else { normal = hct::Vec<T, D>(0); }
			return ScalarFunctionValue<D, T>(dist, normal);
		}
		hct::Vec<T, D> m_center;
	};

	template<unsigned int D, typename T>
	inline
		PointDistanceFunction<D, T>
		point_distance_function(const hct::Vec<T, D>& center)
	{
		return PointDistanceFunction<D, T>(center);
	}

	// ================= plane ==================
	template<unsigned int _D, typename _T = double>
	struct PlaneFunction : public IScalarFunction<_D, _T>
	{
		using T = _T;
		static constexpr unsigned int D = _D;
		inline PlaneFunction(const hct::Vec<T, D + 1>& p) : m_plane(p) {}
		inline ScalarFunctionValue<D, T> operator () (const hct::Vec<T, D>& p) const override final
		{
			// m_plane is truncated to its n-1 first coefficients, that is, its normal vector
			return ScalarFunctionValue<D, T>(p.dot(m_plane) + m_plane.val, m_plane);
		}
		hct::Vec<T, D + 1> m_plane;
	};

	template<unsigned int D, typename T>
	inline
	PlaneFunction<D - 1, T>
	plane_function(const hct::Vec<T, D>& planeEq)
	{
		return PlaneFunction<D - 1, T>(planeEq);
	}

	// ================ Negation operator ==================
	template<typename FuncT>
	struct NegateFunction : public IScalarFunction<FuncT::D, typename FuncT::T>
	{
		using T = typename FuncT::T;
		static constexpr unsigned int D = FuncT::D;
		inline NegateFunction(FuncT f) : m_f(f) {}
		inline ScalarFunctionValue<D, T> operator () (const hct::Vec<T, D>& p) const override final
		{
			return -m_f(p);
		}
		FuncT m_f;
	};

	template<typename FuncT>
	inline NegateFunction<FuncT> negate_function(FuncT f)
	{
		return NegateFunction<FuncT>(f);
	}

	// ================ add operator ==================
	template<typename Func1T, typename Func2T, bool is_valid = (Func1T::D == Func2T::D) && std::is_same<typename Func1T::T, typename Func2T::T>::value > struct AddFunction {};
	template<typename Func1T, typename Func2T>
	struct AddFunction<Func1T, Func2T, true> : public IScalarFunction<Func1T::D, typename Func1T::T>
	{
		using T = typename Func1T::T;
		static constexpr unsigned int D = Func1T::D;
		inline AddFunction(Func1T f1, Func2T f2) : m_f1(f1), m_f2(f2) {}
		inline ScalarFunctionValue<D, T> operator () (const hct::Vec<T, D>& p) const override final
		{
			return  m_f1(p) + m_f2(p);
		}
		Func1T m_f1;
		Func2T m_f2;
	};

	template<typename Func1T, typename Func2T>
	inline AddFunction<Func1T, Func2T> add_function(Func1T f1, Func2T f2)
	{
		return AddFunction<Func1T, Func2T>(f1, f2);
	}


	template<typename CondFuncT,typename Func1T, typename Func2T,
		bool is_valid = (Func1T::D == Func2T::D) && (CondFuncT::D== Func1T::D) && std::is_same<typename CondFuncT::T,bool>::value && std::is_same<typename Func1T::T, typename Func2T::T>::value >
	struct ConditionalFunction {};

	template<typename CondFuncT, typename Func1T, typename Func2T>
	struct ConditionalFunction<CondFuncT,Func1T, Func2T, true> : public IScalarFunction<Func1T::D, typename Func1T::T>
	{
		using T = typename Func1T::T;
		static constexpr unsigned int D = Func1T::D;
		inline ConditionalFunction(CondFuncT c, Func1T f1, Func2T f2) : m_cond(c), m_f1(f1), m_f2(f2) {}
		inline ScalarFunctionValue<D, T> operator () (const hct::Vec<T, D>& p) const override final
		{
			if (m_cond(p))
			{
				return m_f1(p);
			}
			else
			{
				return m_f2(p);
			}
		}
		CondFuncT m_cond;
		Func1T m_f1;
		Func2T m_f2;
	};


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
	struct CSGUnion<Func1T, Func2T, true> : public IScalarFunction<Func1T::D, typename Func1T::T>
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
	struct CSGIntersection<Func1T, Func2T, true> : public IScalarFunction<Func1T::D, typename Func1T::T>
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
		return CSGIntersection<Func1T, NegateFunction<Func2T> >(f1, NegateFunction<Func2T>(f2));
	}

	template<unsigned int D, typename T>
	inline
		AddFunction< PointDistanceFunction<D, T>, ConstantFunction<D, T> >
		csg_sphere(const hct::Vec<T, D>& center, const T& radius)
	{
		return add_function(PointDistanceFunction<D, T>(center), ConstantFunction<D, T>(-radius));
	}

	// ============= boolean inside operator =============
	// true means inside, false means outside
	template<typename FuncT>
	struct CSGInside
	{
		using ScalarT = typename FuncT::T;
		using T = bool;
		static constexpr unsigned int D = FuncT::D;
		inline T operator () (const hct::Vec<ScalarT, D>& p) const { return m_f(p).m_value < 0; }
		FuncT m_f;
	};

	template<typename FuncT>
	inline CSGInside<FuncT> csg_inside(FuncT f)
	{
		return CSGInside<FuncT>{f};
	}

}
