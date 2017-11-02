# more-fixed-cpp

Single-header C++11 fixed-point arithmetic

- Use `more::fixed16` as a drop-in replacement for `float` in existing code.
- No dependencies needed outside of the C++ standard library.
- Reasonable speed (but aggressive optimization is not a current goal).

This library may be useful if you'd like your mathematical calculations to be
exactly reproducible (usually not the case with floating-point arithmetic).
For example, in a game physics engine, you could run the same physics on two
different machines and be sure of getting identical results.

## How to use it

`#include "more_fixed/more_fixed.h"`

Use one of these types instead of `float`:

- `more::fixed16`: 16.16 bit fixed point. Aborts on overflow in debug builds.
- `more::fixed16_safe`: as above, but always aborts on overflow.
- `more::fixed16_fast`: as above, but ignores overflow.

Here's my Box2D fork that works with fixed-point:
https://github.com/more-please/liquidfun

On x86_64, `fixed16_fast` seems to be about the same speed as `float`, while
`fixed16_safe` is 1.5x--2x slower.

## Things to watch out for

In general, this is still a work in progress, so use with caution. I don't have
exhaustive unit tests yet, I'm just doing ad-hoc integration testing.

### Build system

I'm using Clang with C++11 support. Other than fixing as many warnings as
possible, I haven't put any special effort into making the code portable.

### Reproducibility

Complete reproducibility is the goal, but I haven't actually checked this yet!

I currently have a cheesy implementation of `sqrt`, `sin`, `cos` etc, which is
to convert values to `float` and just use `math.h`. That may actually be okay,
if it always results in higher-precision results that are then truncated to
`fixed`. (Probably it needs to be `double` rather than `float`, single
precision doesn't have a full 32-bit mantissa.)
