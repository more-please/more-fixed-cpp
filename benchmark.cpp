#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include "more_fixed/more_fixed.h"

using namespace more;
using namespace std;

template <typename T> int mandelbrot(T x0, T y0, int max_iterations)
{
	T x = 0, y = 0;
	for (int i = 0; i < max_iterations; ++i) {
		T _x = x * x - y * y + x0;
		T _y = 2 * x * y + y0;
		if (_x * _x + _y * _y >= 2 * 2) return i;
		x = _x, y = _y;
	}
	return max_iterations;
}

template <typename T> void plot(FILE* out, int max_iterations)
{
	T step = 1 / 16.0;
	for (T y = -1; y <= 1; y += step) {
		for (T x = -2; x <= 1; x += step) {
			int i = mandelbrot(x, y, max_iterations);
			char c = (i == max_iterations) ? '*' : " ."[i % 2];
			fputc(c, out);
		}
		fputc('\n', out);
	}
}

typedef void (*plot_func)(FILE*, int);

struct numeric_type
{
	const char* name;
	const char* help;
	plot_func func;
};

const std::vector<numeric_type> TYPES = {
	{ "float", "32-bit floating point", plot<float> },
	{ "double", "64-bit floating point", plot<double> },
	{ "fixed_safe", "16.16 fixed point, abort on overflow", plot<fixed16_safe> },
	{ "fixed_fast", "16.16 fixed point, no overflow check", plot<fixed16_fast> },
};

void usage(const char* exe)
{
	fprintf(stderr, "Usage: %s <max_iterations> <numeric_type>\n\n", exe);
	fprintf(stderr, "Prints a Mandelbrot set. Available numeric types:\n");
	for (auto& t : TYPES) fprintf(stderr, "  %s: %s\n", t.name, t.help);
	fprintf(stderr, "\n");
}

int main(int argc, const char* argv[])
{
	if (argc != 3) {
		usage(argv[0]);
		return 1;
	}

	char* end;
	long max_iterations = strtol(argv[1], &end, 10);
	if (*end) {
		fprintf(stderr, "** Expected a number but found: '%s'\n\n", argv[1]);
		usage(argv[0]);
		return 1;
	}

	const char* name = argv[2];
	for (auto& t : TYPES)
		if (strcmp(t.name, name) == 0) {
			t.func(stdout, max_iterations);
			return 0;
		}

	fprintf(stderr, "** Expected a numeric type but found: '%s'\n\n", name);
	usage(argv[0]);
	return 1;
}
