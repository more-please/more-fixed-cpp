# more-fixed-cpp

Single-header C++11 fixed-point arithmetic

## What

This template `more::fixed<N>` defines a fixed-point value with `N` bits of
fractional precision, stored in an `int32_t`. Multiplication and division
use `int64_t` internally for maximum precision.

My goal is for `fixed` to be usable as a drop-in replacement for `float` in
existing libraries. I'm currently using it with Box2D. It needs a few hacks
to work properly -- see my fork at https://github.com/more-please/liquidfun.

## How

`#include "more_fixed/more_fixed.h"`

## Why

Floating-point arithmetic is very fast and accurate on modern machines, but
usually the results aren't reproducible (different machines or even different
runs may give different output for identical input). 32-bit fixed-point
arithmetic should be exactly reproducible on all conventional computers.

In Box2D, for example, this means you can run the same physics calculation on
two different machines and be sure of getting the same result.

## Caveats

In general, this is still a work in progress, so use with caution. I don't have
unit tests yet, I'm just doing integration testing (making sure it works in my
Box2D example).

### Overflow

Overflow is currently checked via `assert`. I tried saturating arithmetic but
found it didn't play well with Box2D -- it would give weird results without
warning. Better to avoid overflow entirely!

The "correct" C++ thing to do would be to add an overflow strategy as a template
parameter. I haven't decided on that yet; I feel it complicates the code a lot
for something that may or may not actually be useful. `assert` is working for
my purposes right now.

### Reproducibility

Complete reproducibility is the goal, but I haven't actually checked this yet!

I currently have a cheesy implementation of `sqrt`, `sin`, `cos` etc, which is
to convert values to `float` and just use `math.h`. That may actually be okay,
if it always results in higher-precision results that are then truncated to
`fixed`. (Probably it needs to be `double` rather than `float`, single
precision doesn't have a full 32-bit mantissa.)

### Build system

I'm using Clang with C++11 support. Other than fixing as many warnings as
possible, I haven't put any special effort into making the code portable.

## Non-goals

### Speed

Fixed-point _could_ be faster than floating-point on some hardware, even with
overflow checks, but this is not an explicit goal. It just needs to be fast
enough for general usage. If you want speed and accuracy, use floats. If you
want reproducible results, use fixed.
