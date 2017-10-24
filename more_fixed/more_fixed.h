#ifndef more_fixed_h
#define more_fixed_h

#include <assert.h>
#include <stdint.h>

#include <limits>

namespace more
{
	template <typename T, int FRAC_BITS> class Fixed
	{
		T _value = 0;

	public:
		typedef Fixed<T, FRAC_BITS> This;

		enum
		{
			SCALE = 1 << FRAC_BITS,
			MAX = std::numeric_limits<T>::max() / SCALE,
			MIN = std::numeric_limits<T>::min() / SCALE,

			FRACT_MASK = SCALE - 1,
			TRUNC_MASK = ~FRACT_MASK,
		};

		Fixed() = default;
		Fixed(const This& other) = default;

		template <typename V> Fixed(V val)
		{
			assert(val <= MAX);
			assert(val >= MIN);
			_value = val * SCALE;
		}

		static Fixed from_bits(T bits)
		{
			Fixed result;
			result._value = bits;
			return result;
		}

		T bits() const { return _value; }

		Fixed trunc() const { return _value & TRUNC_MASK; }
		Fixed fract() const { return _value & FRACT_MASK; }
	};

	typedef Fixed<int32_t, 16> fixed16;
}

#endif // more_fixed_h
