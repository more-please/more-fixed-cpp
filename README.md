# more-fixed-cpp

Single-header C++11 fixed-point arithmetic

## What

This template `more::fixed<N>` defines a fixed-point value with `N` bits of
fractional precision, stored in an `int32_t`. Arithmetic uses `int64_t`
internally. Overflow is optionally detected or ignored.

## How

`#include "more_fixed/more_fixed.h"`

Use `more::fixed16` instead of `float`. If you use a mix of floats and
fixed-point values in expressions, the floats will be converted implicitly,
so using float constants is fine.

Here's my Box2D fork that works with fixed-point:
https://github.com/more-please/liquidfun

## Why

Fixed point arithmetic should be exactly reproducible on all conventional
computers (unlike floating point). In Box2D, for example, that would mean you
could run the same physics calculation on two different machines and be sure
of getting identical results.

## Goals

- Should be usable as a drop-in replacement for `float` in existing code.
- No dependencies needed outside of the C++ standard library.
- Reasonable speed (but aggressive optimization is not a current goal).

---

## More info, caveats

In general, this is still a work in progress, so use with caution. I don't have
exhaustive unit tests yet, I'm just doing ad-hoc integration testing.

### Overflow

One of the parameters to the `fixed` template is a function to call when
overflow is detected. The default `fixed16` typedef calls `assert`, meaning
it will abort by default but this can be disabled in release builds by
defining `NDEBUG`.

The `fixed16_safe` type always aborts, and the `fixed16_fast` type always
ignores overflow.

### Reproducibility

Complete reproducibility is the goal, but I haven't actually checked this yet!

I currently have a cheesy implementation of `sqrt`, `sin`, `cos` etc, which is
to convert values to `float` and just use `math.h`. That may actually be okay,
if it always results in higher-precision results that are then truncated to
`fixed`. (Probably it needs to be `double` rather than `float`, single
precision doesn't have a full 32-bit mantissa.)

### Speed

Fixed-point _could_ be faster than floating-point on some hardware, even with
overflow checks, but this is not an explicit goal. I just want it to be fast
enough general usage; no more than 2x slower than float, say. If you want
speed and accuracy, use floats. If you want reproducible results, use fixed.

On x86_64, `fixed16_fast` seems to be about the same speed as `float`, while
`fixed16_safe` is 1.5x--2x slower.

### Build system

I'm using Clang with C++11 support. Other than fixing as many warnings as
possible, I haven't put any special effort into making the code portable.
