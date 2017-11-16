#ifndef more_fixed_h
#define more_fixed_h

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <limits>

namespace more
{
	// -------------------------------------------------------------------------
	// Template for fixed-point values.
	//
	// BITS is the number of fractional bits.
	// ERR is a function to call when overflow is detected.

	template <int BITS, void (*ERR)()> struct fixed;

	// -------------------------------------------------------------------------
	// Standard formats

	inline void fixed_error_ignore() {}
	inline void fixed_error_abort() { abort(); }
	inline void fixed_error_assert()
	{
		bool fixed_point_overflow = false;
		assert(fixed_point_overflow);
	}

	// Fastest option: always ignore overflow.
	typedef fixed<16, fixed_error_ignore> fixed16_fast;

	// Safest option: always abort on overflow.
	typedef fixed<16, fixed_error_abort> fixed16_safe;

	// Default: use assert() so overflow checks can be easily disabled.
	typedef fixed<16, fixed_error_assert> fixed16;

	// -------------------------------------------------------------------------
	// Implementation
	//
	// All fixed-point types are currently 32 bits in size.
	// Arithmetic uses 64 bit precision internally.

	template <int BITS, void (*ERR)()> struct fixed
	{
		static_assert(BITS >= 0, "Can't have negative fractional bits");
		static_assert(BITS <= 32, "Can't have more than 32 fractional bits");

		int32_t _repr;

		static constexpr int SCALE = 1 << BITS;
		static constexpr int32_t MASK = SCALE - 1;

		typedef fixed F;

		int64_t repr64() const { return _repr; }

		static F from_repr64(int64_t repr_with_carry)
		{
			int64_t carry = repr_with_carry >> 31;
			check(carry == 0 || carry == -1);
			return from_repr(int32_t(repr_with_carry));
		}

		static void check(bool condition)
		{
			if (!condition) ERR();
		}

	public:
		typedef int32_t repr_t;

		fixed() = default;
		fixed(const F&) = default;

		template <typename T> fixed(T value)
		{
			check(isfinite(value));
			T lo = T(limits::min());
			T hi = T(limits::max());
			check(value <= hi);
			check(value >= lo);
			_repr = repr_t(value * SCALE);
		}

		template <typename T> F& operator=(T value)
		{
			T lo = T(limits::min());
			T hi = T(limits::max());
			check(value <= hi);
			check(value >= lo);
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

		repr_t repr() const { return _repr; }

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

		F operator+() const { return *this; }
		F operator-() const
		{
			check(_repr != repr_limits::min());
			return from_repr(-_repr);
		}
		F operator+(const F& rhs) const
		{
			return from_repr64(repr64() + rhs._repr);
		}
		F operator-(const F& rhs) const
		{
			return from_repr64(repr64() - rhs._repr);
		}
		F operator*(const F& rhs) const
		{
			return from_repr64((repr64() * rhs._repr) / SCALE);
		}
		F operator/(const F& rhs) const
		{
			return from_repr64((repr64() * SCALE) / rhs._repr);
		}

		bool operator<(const F& rhs) const { return _repr < rhs._repr; }
		bool operator<=(const F& rhs) const { return _repr <= rhs._repr; }
		bool operator>(const F& rhs) const { return _repr > rhs._repr; }
		bool operator>=(const F& rhs) const { return _repr >= rhs._repr; }
		bool operator==(const F& rhs) const { return _repr == rhs._repr; }
		bool operator!=(const F& rhs) const { return _repr != rhs._repr; }

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
			static constexpr F min() { return from_repr(repr_limits::min()); }
			static constexpr F max() { return from_repr(repr_limits::max()); }
			static constexpr F epsilon() { return from_repr(1); }
		};

		// ---------------------------------------------------------------------
		// math.h

		static F fabs(F f) { return (f < 0) ? -f : f; }
		static F floor(F f) { return from_repr(f._repr & ~MASK); }
		static F ceil(F f) { return from_repr64((f.repr64() + MASK) & ~MASK); }
		static F trunc(F f) { return (f < 0) ? ceil(f) : floor(f); }

		static F sqrt(F f) { return ::sqrt(double(f)); }
		static F sin(F f) { return ::sin(double(f)); }
		static F cos(F f) { return ::cos(double(f)); }
		static F tan(F f) { return ::tan(double(f)); }
		static F exp(F f) { return ::exp(double(f)); }
		static F atan2(F a, F b) { return ::atan2(double(a), double(b)); }
	};

// -----------------------------------------------------------------------------
// Implicit conversions for "float (op) fixed16" expressions

#define MORE_FIXED__OP(OP)                                                     \
	template <typename T, int B, void (*E)()>                                  \
	fixed<B, E> operator OP(T lhs, fixed<B, E> rhs)                            \
	{                                                                          \
		return fixed<B, E>(lhs) OP rhs;                                        \
	}

#define MORE_FIXED__CMP(CMP)                                                   \
	template <typename T, int B, void (*E)()>                                  \
	bool operator CMP(T lhs, fixed<B, E> rhs)                                  \
	{                                                                          \
		return fixed<B, E>(lhs) CMP rhs;                                       \
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

#define MORE_FIXED__MATH(MATH)                                                 \
	template <int B, void (*E)()> fixed<B, E> MATH(fixed<B, E> f)              \
	{                                                                          \
		return fixed<B, E>::MATH(f);                                           \
	}                                                                          \
	template <int B, void (*E)()> fixed<B, E> MATH##f(fixed<B, E> f)           \
	{                                                                          \
		return fixed<B, E>::MATH(f);                                           \
	}

#define MORE_FIXED__MATH2(MATH)                                                \
	template <int B, void (*E)()>                                              \
	fixed<B, E> MATH(fixed<B, E> a, fixed<B, E> b)                             \
	{                                                                          \
		return fixed<B, E>::MATH(a, b);                                        \
	}                                                                          \
	template <int B, void (*E)()>                                              \
	fixed<B, E> MATH##f(fixed<B, E> a, fixed<B, E> b)                          \
	{                                                                          \
		return fixed<B, E>::MATH(a, b);                                        \
	}

	MORE_FIXED__MATH(fabs)
	MORE_FIXED__MATH(sin)
	MORE_FIXED__MATH(cos)
	MORE_FIXED__MATH(tan)
	MORE_FIXED__MATH2(atan2)
	MORE_FIXED__MATH(sqrt)
	MORE_FIXED__MATH(exp)
	MORE_FIXED__MATH(ceil)
	MORE_FIXED__MATH(floor)

#undef MORE_FIXED__MATH
#undef MORE_FIXED__MATH2

	// -------------------------------------------------------------------------
	// Classification functions

	template <int B, void (*E)()> bool isfinite(fixed<B, E>) { return true; }
	template <int B, void (*E)()> bool isinf(fixed<B, E>) { return false; }
	template <int B, void (*E)()> bool isnan(fixed<B, E>) { return false; }
	template <int B, void (*E)()> bool isnormal(fixed<B, E> f)
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
