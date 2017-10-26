#ifndef more_fixed_h
#define more_fixed_h

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include <limits>

namespace more
{
	template <int BITS> class fixed
	{
		int32_t _repr;

		static constexpr int SCALE = 1 << BITS;

		typedef fixed F;

	public:
		typedef int32_t repr_t;

		fixed() = default;
		fixed(const F&) = default;

		template <typename T> fixed(T value)
		{
			if (value > T(limits::max()))
				value = T(limits::max());
			if (value < T(limits::min()))
				value = T(limits::min());
			_repr = repr_t(value * SCALE);
		}

		template <typename T> F& operator=(T value)
		{
			if (value > T(limits::max()))
				value = T(limits::max());
			if (value < T(limits::min()))
				value = T(limits::min());
			return set_repr(value * SCALE);
		}

		template <typename T> explicit operator T() const
		{
			return T(_repr) / T(SCALE);
		}

		// ---------------------------------------------------------------------
		// Bitwise accessors

		repr_t repr() const
		{
			return _repr;
		}

		static F from_repr(repr_t repr)
		{
			F result;
			result._repr = repr;
			return result;
		}

		F& set_repr(repr_t repr)
		{
			_repr = repr;
			return *this;
		}

		// ---------------------------------------------------------------------
		// Operators

		F operator+() const
		{
			return *this;
		}

		F operator-() const
		{
			return from_repr(-_repr);
		}

		F operator+(F rhs) const
		{
			return double(*this) + double(rhs);
		}
		F operator-(F rhs) const
		{
			return double(*this) - double(rhs);
		}
		F operator*(F rhs) const
		{
			return double(*this) * double(rhs);
		}
		F operator/(F rhs) const
		{
			return double(*this) / double(rhs);
		}

		F& operator+=(F rhs)
		{
			*this = double(*this) + double(rhs);
			return *this;
		}
		F& operator-=(F rhs)
		{
			*this = double(*this) - double(rhs);
			return *this;
		}
		F& operator*=(F rhs)
		{
			*this = double(*this) * double(rhs);
			return *this;
		}
		F& operator/=(F rhs)
		{
			*this = double(_repr) / double(rhs._repr);
			return *this;
		}

		bool operator<(F rhs) const
		{
			return _repr < rhs._repr;
		}
		bool operator<=(F rhs) const
		{
			return _repr <= rhs._repr;
		}
		bool operator>(F rhs) const
		{
			return _repr > rhs._repr;
		}
		bool operator>=(F rhs) const
		{
			return _repr >= rhs._repr;
		}
		bool operator==(F rhs) const
		{
			return _repr == rhs._repr;
		}
		bool operator!=(F rhs) const
		{
			return _repr != rhs._repr;
		}

		// ---------------------------------------------------------------------
		// numeric_limits

		typedef std::numeric_limits<repr_t> repr_limits;

		struct limits
		{
			static constexpr bool is_specialized = true;
			static constexpr F min()
			{
				return from_repr(repr_limits::min());
			}
			static constexpr F max()
			{
				return from_repr(repr_limits::max());
			}
			static constexpr F epsilon()
			{
				return from_repr(1);
			}
		};

		// ---------------------------------------------------------------------
		// math.h

		static F sin(F f)
		{
			return ::sin(double(f));
		}
		static F cos(F f)
		{
			return ::cos(double(f));
		}
		static F sqrt(F f)
		{
			return ::sqrt(double(f));
		}
		static F exp(F f)
		{
			return ::exp(double(f));
		}
		static F atan2(F a, F b)
		{
			return ::atan2(double(a), double(b));
		}
		static F floor(F f)
		{
			return ::floor(double(f));
		}
		static F ceil(F f)
		{
			return ::ceil(double(f));
		}
	};

// -----------------------------------------------------------------------------
// Implicit conversions for "double (op) fixed16" expressions

#define MORE_FIXED__OP(OP)                                                     \
	template <typename T, int B> fixed<B> operator OP(T lhs, fixed<B> rhs)     \
	{                                                                          \
		return fixed<B>(lhs) OP rhs;                                           \
	}

#define MORE_FIXED__CMP(CMP)                                                   \
	template <typename T, int B> bool operator CMP(T lhs, fixed<B> rhs)        \
	{                                                                          \
		return fixed<B>(lhs) CMP rhs;                                          \
	}

	MORE_FIXED__OP(+)
	MORE_FIXED__OP(-)
	MORE_FIXED__OP(*)
	MORE_FIXED__OP(/)

	MORE_FIXED__CMP(<)
	MORE_FIXED__CMP(<=)
	MORE_FIXED__CMP(>)
	MORE_FIXED__CMP(>=)

#undef MORE_FIXED__OP
#undef MORE_FIXED__CMP

// -------------------------------------------------------------------------
// Forward math.h functions to class

#define MORE_FIXED__TRIG(TRIG)                                                 \
	template <int N> fixed<N> TRIG(fixed<N> f)                                 \
	{                                                                          \
		return fixed<N>::TRIG(f);                                              \
	}                                                                          \
	template <int N> fixed<N> TRIG##f(fixed<N> f)                              \
	{                                                                          \
		return fixed<N>::TRIG(f);                                              \
	}

#define MORE_FIXED__TRIG2(TRIG)                                                \
	template <int N> fixed<N> TRIG(fixed<N> a, fixed<N> b)                     \
	{                                                                          \
		return fixed<N>::TRIG(a, b);                                           \
	}                                                                          \
	template <int N> fixed<N> TRIG##f(fixed<N> a, fixed<N> b)                  \
	{                                                                          \
		return fixed<N>::TRIG(a, b);                                           \
	}

	MORE_FIXED__TRIG(sin)
	MORE_FIXED__TRIG(cos)
	MORE_FIXED__TRIG(tan)
	MORE_FIXED__TRIG2(atan2)
	MORE_FIXED__TRIG(sqrt)
	MORE_FIXED__TRIG(exp)
	MORE_FIXED__TRIG(ceil)
	MORE_FIXED__TRIG(floor)

#undef MORE_FIXED__TRIG
#undef MORE_FIXED__TRIG2

	// -------------------------------------------------------------------------
	// Classification functions

	template <int N> bool isfinite(fixed<N>)
	{
		return true;
	}
	template <int N> bool isinf(fixed<N>)
	{
		return false;
	}
	template <int N> bool isnan(fixed<N>)
	{
		return false;
	}
	template <int N> bool isnormal(fixed<N> f)
	{
		return f.repr() != 0;
	}

	// -------------------------------------------------------------------------
	// Typedefs for standard formats

	typedef fixed<16> fixed16;
}

namespace std
{
	template <>
	struct numeric_limits<more::fixed16> : public more::fixed16::limits
	{
	};
}

#endif // more_fixed_h
