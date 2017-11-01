#ifndef more_fixed_h
#define more_fixed_h

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include <limits>

namespace more
{
	// -------------------------------------------------------------------------
	// Standard formats
	//
	// All fixed-point types are currently 32 bits in size.
	// Multiplication and division is done with 64 bit precision internally.
	// BITS is the number of fractional bits, so fixed<16> is 16.16 fixed point.

	template <int BITS> struct fixed;

	typedef fixed<16> fixed16;

	// -------------------------------------------------------------------------
	// Implementation

	template <int BITS> struct fixed
	{
		int32_t _repr;

		static constexpr int SCALE = 1 << BITS;

		typedef fixed F;

		int64_t repr64() const
		{
			return _repr;
		}

		static F from_repr64_check(int64_t repr_with_carry)
		{
			uint32_t carry = uint64_t(repr_with_carry) >> 32;
			assert(carry == 0 || carry == 0xffffffffu);
			return from_repr(int32_t(repr_with_carry));
		}

	public:
		typedef int32_t repr_t;

		fixed() = default;
		fixed(const F&) = default;

		template <typename T> fixed(T value)
		{
			T lo = T(limits::min());
			T hi = T(limits::max());
			assert(value <= hi);
			assert(value >= lo);
			_repr = repr_t(value * SCALE);
		}

		template <typename T> F& operator=(T value)
		{
			T lo = T(limits::min());
			T hi = T(limits::max());
			assert(value <= hi);
			assert(value >= lo);
			return set_repr(value * SCALE);
		}

		template <typename T> explicit operator T() const
		{
			if (std::numeric_limits<T>::is_integer)
				return T(_repr / SCALE);
			else
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
			assert(_repr != repr_limits::min());
			return from_repr(-_repr);
		}
		F operator+(const F& rhs) const
		{
			return from_repr64_check(_repr + rhs._repr);
		}
		F operator-(const F& rhs) const
		{
			return from_repr64_check(_repr - rhs._repr);
		}
		F operator*(const F& rhs) const
		{
			return from_repr((repr64() * rhs._repr) / SCALE);
		}
		F operator/(const F& rhs) const
		{
			return from_repr((repr64() * SCALE) / rhs._repr);
		}
		bool operator<(const F& rhs) const
		{
			return _repr < rhs._repr;
		}
		bool operator<=(const F& rhs) const
		{
			return _repr <= rhs._repr;
		}
		bool operator>(const F& rhs) const
		{
			return _repr > rhs._repr;
		}
		bool operator>=(const F& rhs) const
		{
			return _repr >= rhs._repr;
		}
		bool operator==(const F& rhs) const
		{
			return _repr == rhs._repr;
		}
		bool operator!=(const F& rhs) const
		{
			return _repr != rhs._repr;
		}

		F& operator+=(const F& rhs)
		{
			*this = *this + rhs;
			return *this;
		}
		F& operator-=(const F& rhs)
		{
			*this = *this - rhs;
			return *this;
		}
		F& operator*=(const F& rhs)
		{
			*this = *this * rhs;
			return *this;
		}
		F& operator/=(const F& rhs)
		{
			*this = *this / rhs;
			return *this;
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

		static F fabs(const F& f)
		{
			return ::fabsf(float(f));
		}
		static F sin(const F& f)
		{
			return ::sinf(float(f));
		}
		static F cos(const F& f)
		{
			return ::cosf(float(f));
		}
		static F sqrt(const F& f)
		{
			return ::sqrtf(float(f));
		}
		static F exp(const F& f)
		{
			return ::expf(float(f));
		}
		static F atan2(const F& a, const F& b)
		{
			return ::atan2f(float(a), float(b));
		}
		static F floor(const F& f)
		{
			return ::floorf(float(f));
		}
		static F ceil(const F& f)
		{
			return ::ceilf(float(f));
		}
	};

// -----------------------------------------------------------------------------
// Implicit conversions for "float (op) fixed16" expressions

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

// -----------------------------------------------------------------------------
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

	MORE_FIXED__TRIG(fabs)
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
}

namespace std
{
	template <>
	struct numeric_limits<more::fixed16> : public more::fixed16::limits
	{
	};
}

#endif // more_fixed_h
