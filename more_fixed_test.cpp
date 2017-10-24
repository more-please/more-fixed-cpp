#include "more_fixed/more_fixed.h"

#include <assert.h>
#include <stdio.h>

using namespace more;

int main(int argc, const char* argv[])
{
	printf("fixed16 range: %d to %d\n", fixed16::MIN, fixed16::MAX);

	printf("half\n");
	fixed16 half = 0.5f;

	printf("a\n");
	fixed16 a = 0.5f * half;
	printf("b\n");
	fixed16 b = half * 0.5f;
	printf("c\n");
	fixed16 c = half * half;

	printf("All tests passed!\n");
	return 0;
}
