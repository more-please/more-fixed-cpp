#include "../include/more_fixed.h"

#include <assert.h>
#include <stdio.h>

using namespace more;

static int overflows = 0;

void count_overflows() { ++overflows; }

typedef more::fixed<16, count_overflows> count16;

int main(int argc, const char* argv[])
{
	printf(
		"fixed16 range: %f to %f\n",
		double(fixed16::limits::max()),
		double(fixed16::limits::min()));

	// We can mix fixed variables with float constants

	const count16 half = 0.5f;
	const count16 quarter = half * half;

	assert(half == 0.5f);
	assert(half != quarter);
	assert(quarter == half * 0.5f);
	assert(quarter == 0.5f * 0.5f);

	// Basic check for overflow detection

	const count16 hi = count16::limits::max();
	const count16 lo = count16::limits::min();

	auto a = hi - 1;
	auto b = lo + 1;
	assert(overflows == 0);

	a = hi + 1;
	assert(overflows == 1);
	b = lo - 1;
	assert(overflows == 2);

	a = lo * 1.01;
	assert(overflows == 3);
	b = hi / 0.99;
	assert(overflows == 4);

	printf("All tests passed!\n");
	return 0;
}
