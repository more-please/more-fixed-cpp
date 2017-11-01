#include "more_fixed/more_fixed.h"

#include <assert.h>
#include <stdio.h>

using namespace more;

int main(int argc, const char* argv[])
{
	printf(
		"fixed16 range: %f to %f\n",
		double(fixed16::limits::max()),
		double(fixed16::limits::min()));

	fixed16 half = 0.5f;
	fixed16 quarter = half * half;

	assert(half == 0.5f);
	assert(half != quarter);
	assert(quarter == half * 0.5f);
	assert(quarter == 0.5f * 0.5f);

	printf("All tests passed!\n");
	return 0;
}
