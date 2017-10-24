#ifndef more_fixed_h
#define more_fixed_h

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include <limits>

namespace more
{
	template <typename T, int BITS> class Fixed
	{
		T _val;

	public:
		typedef Fixed<T, BITS> F;

		enum
		{
			SCALE = 1 << BITS,
			MAX = std::numeric_limits<T>::max() / SCALE,
			MIN = std::numeric_limits<T>::min() / SCALE,

			FRACT_MASK = SCALE - 1,
			TRUNC_MASK = ~FRACT_MASK,
		};

		Fixed() = default;
		Fixed(const Fixed&) = default;

		static double check(double val)
		{
			static double hi = 0;
			static double lo = 0;
			if (val > hi)
			{
				hi = val;
				printf("High water mark: %f\n", hi);
			}
			if (val < lo)
			{
				lo = val;
				printf("Low water mark: %f\n", lo);
			}
			if (val > MAX)
			{
				return MAX;
			}
			if (val <= MIN)
			{
				return MIN;
			}
			return val;
		}

		template <typename V> Fixed(V val)
		{
			_val = check(val) * SCALE;
		}

		template <typename V> F& operator=(V val)
		{
			return set_bits(val * SCALE);
		}

		// ---------------------------------------------------------------------
		// Bitwise accessors

		T bits() const
		{
			return _val;
		}
		static F from_bits(T bits)
		{
			F result;
			result._val = bits;
			return result;
		}

		F& set_bits(T bits)
		{
			_val = bits;
			return *this;
		}

		// ---------------------------------------------------------------------
		// Conversions

		explicit operator float() const
		{
			return _val / float(SCALE);
		}
		explicit operator double() const
		{
			return _val / double(SCALE);
		}
		explicit operator long long() const
		{
			return _val >> BITS;
		}
		explicit operator long() const
		{
			return _val >> BITS;
		}
		explicit operator int() const
		{
			return _val >> BITS;
		}
		explicit operator short() const
		{
			return _val >> BITS;
		}
		explicit operator char() const
		{
			return _val >> BITS;
		}
		explicit operator unsigned long long() const
		{
			return _val >> BITS;
		}
		explicit operator unsigned long() const
		{
			return _val >> BITS;
		}
		explicit operator unsigned int() const
		{
			return _val >> BITS;
		}
		explicit operator unsigned short() const
		{
			return _val >> BITS;
		}
		explicit operator unsigned char() const
		{
			return _val >> BITS;
		}

		// ---------------------------------------------------------------------
		// Operators

		F operator-() const
		{
			return -double(*this);
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
			*this = double(_val) / double(rhs._val);
			return *this;
		}

		bool operator<(F rhs) const
		{
			return _val < rhs._val;
		}
		bool operator<=(F rhs) const
		{
			return _val <= rhs._val;
		}
		bool operator>(F rhs) const
		{
			return _val > rhs._val;
		}
		bool operator>=(F rhs) const
		{
			return _val >= rhs._val;
		}
		bool operator==(F rhs) const
		{
			return _val == rhs._val;
		}
		bool operator!=(F rhs) const
		{
			return _val != rhs._val;
		}

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
			return from_bits(f._val & TRUNC_MASK);
		}
		static F ceil(F f)
		{
			return from_bits(f._val & TRUNC_MASK);
		}
	};

	typedef Fixed<int32_t, 16> fixed16;

// -----------------------------------------------------------------------------
// Implicit conversions for "float (op) fixed16" expressions

#define MORE_FIXED__OP(OP)                                                     \
	template <typename V, typename T, int B>                                   \
	Fixed<T, B> operator OP(V lhs, Fixed<T, B> rhs)                            \
	{                                                                          \
		return Fixed<T, B>(lhs) OP rhs;                                        \
	}

#define MORE_FIXED__CMP(CMP)                                                   \
	template <typename V, typename T, int B>                                   \
	bool operator CMP(V lhs, Fixed<T, B> rhs)                                  \
	{                                                                          \
		return Fixed<T, B>(lhs) CMP rhs;                                       \
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

	template <typename T> T sinf(T val)
	{
		return T::sin(val);
	}
	template <typename T> T cosf(T val)
	{
		return T::cos(val);
	}
	template <typename T> T atan2f(T a, T b)
	{
		return T::atan2(a, b);
	}
	template <typename T> T sqrtf(T val)
	{
		return T::sqrt(val);
	}
	template <typename T> T expf(T val)
	{
		return T::exp(val);
	}
	template <typename T> T floorf(T val)
	{
		return T::floor(val);
	}
	template <typename T> T ceilf(T val)
	{
		return T::ceil(val);
	}
}

namespace std
{
	template <> class numeric_limits<more::fixed16>
	{
		static constexpr bool is_specialized = true;
		static constexpr more::fixed16 min()
		{
			return more::fixed16::MIN;
		}
		static constexpr more::fixed16 max()
		{
			return more::fixed16::MAX;
		}
		static constexpr more::fixed16 epsilon()
		{
			return more::fixed16::from_bits(1);
		}
	};

	bool isfinite(more::fixed16)
	{
		return false;
	}
}

#endif // more_fixed_h
