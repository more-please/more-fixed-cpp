#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>

#include "../include/more_fixed.h"

using namespace more;
using namespace std;

// -----------------------------------------------------------------------------
// Base class for tests

struct Test
{
	Test(const char* name, int bits, mutex& mutex)
		: _name(name), _bits(bits), _mutex(mutex)
	{}

	virtual ~Test() = default;

	// Return true on success
	virtual bool test_all(int step) = 0;

	void print(const char* format, ...)
	{
		unique_lock<mutex> lock(_mutex);
		printf("%8s.%02d: ", _name, _bits);

		va_list args;
		va_start(args, format);
		vprintf(format, args);
		va_end(args);

		putchar('\n');
	}

private:
	const char* _name;
	int _bits;
	mutex& _mutex;
};

// -----------------------------------------------------------------------------
// Overflow detector

thread_local bool _overflow = false;

void overflow() { _overflow = true; }

bool get_overflow()
{
	bool result = _overflow;
	_overflow = false;
	return result;
}

// -----------------------------------------------------------------------------
// Test implementation for unary functions

template <typename FIXED, double (*DFUNC)(double), FIXED (*FFUNC)(FIXED)>
struct TestFunc : public Test
{
	TestFunc(const char* name, mutex& mutex) : Test(name, FIXED::BITS, mutex) {}

	virtual bool test_all(int step)
	{
		bool ok = test(0);
		int32_t i;
		for (i = 0; ok && i < 4; ++i) {
			ok = ok && test_repr(INT32_MIN + i);
			ok = ok && test_repr(INT32_MAX - i);
			ok = ok && test_repr(i + 1);
			ok = ok && test_repr(-i - 1);
			ok = ok && test(i + 1);
			ok = ok && test(-i - 1);
		}
		for (i = INT32_MIN + step; ok && i < INT32_MAX - step; i += step) {
			ok = ok && test_repr(i);
		}
		print("%s", ok ? "ok" : "FAILED");
		return ok;
	}

private:
	bool test_repr(int32_t repr) { return test(FIXED::from_repr(repr)); }

	bool test(FIXED fval)
	{
		const double nan = numeric_limits<double>::quiet_NaN();
		const double dval = double(fval);
		const double exact = DFUNC(dval);

		int overflow = 0;
		get_overflow();

		FIXED fexpected = FIXED(exact);
		if (get_overflow()) ++overflow;

		FIXED factual = FFUNC(fval);
		if (get_overflow()) ++overflow;

		int err = factual.repr() - fexpected.repr();

		if (overflow) {
			if (overflow != 2) {
				double expected = double(FIXED(exact));
				if (get_overflow()) expected = nan;

				double actual = double(FFUNC(fval));
				if (get_overflow()) actual = nan;

				log_error(dval, expected, actual);
				return false;
			}
		}
		else if (abs(err) > 1)
		{
			log_error(dval, double(fexpected), double(factual));
			return false;
		}

		return true;
	}

	void log_error(double val, double expected, double actual)
	{
		print("%f: expected %f, got %f", val, expected, actual);
	}
};

// -----------------------------------------------------------------------------
// Test implementation for binary functions

template <typename FIXED, double (*DFUNC)(double, double), FIXED (*FFUNC)(FIXED, FIXED)>
struct TestFunc2 : public Test
{
	TestFunc2(const char* name, mutex& mutex) : Test(name, FIXED::BITS, mutex)
	{}

	virtual bool test_all(int step)
	{
		vector<int32_t> reprs;

		int32_t i;
		for (i = 0; i < 4; ++i) {
			reprs.push_back(INT32_MIN + i);
			reprs.push_back(INT32_MAX - i);
			reprs.push_back(i);
			reprs.push_back(-i);
			reprs.push_back(FIXED(i).repr());
			reprs.push_back(FIXED(-i).repr());
		}

		bool ok = true;
		for (auto& a : reprs)
			for (auto& b : reprs) ok = ok && test_repr(a, b);

		uint32_t j = 0;
		for (i = INT32_MIN + step; ok && i < INT32_MAX - step; i += step) {
			j += 2654435789u; // Prime close to UINT32_MAX * phi
			ok = ok && test_repr(i, int32_t(j));
		}

		print("%s", ok ? "ok" : "FAILED");
		return ok;
	}

private:
	bool test_repr(int32_t a, int32_t b)
	{
		return test(FIXED::from_repr(a), FIXED::from_repr(b));
	}

	bool test(FIXED fa, FIXED fb)
	{
		const double nan = numeric_limits<double>::quiet_NaN();
		const double da = double(fa);
		const double db = double(fb);
		const double exact = DFUNC(da, db);

		int overflow = 0;
		get_overflow();

		FIXED fexpected = FIXED(exact);
		if (get_overflow()) ++overflow;

		FIXED factual = FFUNC(fa, fb);
		if (get_overflow()) ++overflow;

		int err = factual.repr() - fexpected.repr();

		if (overflow) {
			if (overflow != 2) {
				double expected = double(FIXED(exact));
				if (get_overflow()) expected = nan;

				double actual = double(FFUNC(fa, fb));
				if (get_overflow()) actual = nan;

				log_error(da, db, expected, actual);
				return false;
			}
		}
		else if (abs(err) > 1)
		{
			log_error(da, db, double(fexpected), double(factual));
			return false;
		}

		return true;
	}

	void log_error(double a, double b, double expected, double actual)
	{
		print("%f, %f: expected %f, got %f", a, b, expected, actual);
	}
};

// -----------------------------------------------------------------------------
// Test implementation for boolean operators

template <typename FIXED, bool (*DFUNC)(double, double), bool (*FFUNC)(FIXED, FIXED)>
struct TestFuncB : public Test
{
	TestFuncB(const char* name, mutex& mutex) : Test(name, FIXED::BITS, mutex)
	{}

	virtual bool test_all(int step)
	{
		vector<int32_t> reprs;

		int32_t i;
		for (i = 0; i < 4; ++i) {
			reprs.push_back(INT32_MIN + i);
			reprs.push_back(INT32_MAX - i);
			reprs.push_back(i);
			reprs.push_back(-i);
			reprs.push_back(FIXED(i).repr());
			reprs.push_back(FIXED(-i).repr());
		}

		bool ok = true;
		for (auto& a : reprs)
			for (auto& b : reprs) ok = ok && test_repr(a, b);

		uint32_t j = 0;
		for (i = INT32_MIN + step; ok && i < INT32_MAX - step; i += step) {
			j += 2654435789u; // Prime close to UINT32_MAX * phi
			ok = ok && test_repr(i, int32_t(j));
		}

		print("%s", ok ? "ok" : "FAILED");
		return ok;
	}

private:
	bool test_repr(int32_t a, int32_t b)
	{
		return test(FIXED::from_repr(a), FIXED::from_repr(b));
	}

	bool test(FIXED fa, FIXED fb)
	{
		const double da = double(fa);
		const double db = double(fb);

		get_overflow();

		const bool expected = DFUNC(da, db);
		const bool actual = FFUNC(fa, fb);

		if (get_overflow()) {
			print(
				"%f, %f: expected %s, got NaN",
				da,
				db,
				expected ? "true" : "false");
			return false;
		}
		if (actual != expected) {
			log_error(da, db, expected, actual);
			return false;
		}
		return true;
	}

	void log_error(double a, double b, bool expected, bool actual)
	{
		print(
			"%f, %f: expected %s, got %s",
			a,
			b,
			expected ? "true" : "false",
			actual ? "true" : "false");
	}
};

// -----------------------------------------------------------------------------
// Test runner - pulls tests from the queue, designed to work in a thread pool

mutex _mutex{};
condition_variable _cond{};
int _finished = 0;
bool _failed = false;

extern deque<Test*> _tests;

Test* get_test()
{
	unique_lock<mutex> lock(_mutex);
	if (_failed || _tests.empty()) {
		return NULL;
	}
	Test* result = _tests.front();
	_tests.pop_front();
	return result;
}

void run_tests_worker(int step)
{
	for (Test* test = get_test(); test; test = get_test()) {
		bool success = test->test_all(step);

		unique_lock<mutex> lock(_mutex);
		if (!success) _failed = true;
		++_finished;
		_cond.notify_one();
	}
}

// -----------------------------------------------------------------------------
// Function wrappers for binary operators

template <typename T> T negate(T a) { return -a; }
template <typename T> T plus(T a, T b) { return a + b; }
template <typename T> T minus(T a, T b) { return a - b; }
template <typename T> T times(T a, T b) { return a * b; }
template <typename T> T divide(T a, T b) { return a / b; }

template <typename T> bool equal(T a, T b) { return a == b; }
template <typename T> bool neq(T a, T b) { return a != b; }
template <typename T> bool lower(T a, T b) { return a < b; }
template <typename T> bool leq(T a, T b) { return a <= b; }
template <typename T> bool greater(T a, T b) { return a > b; }
template <typename T> bool geq(T a, T b) { return a >= b; }

// -----------------------------------------------------------------------------
// The tests. One Test object per function / precision combination.

// Unary functions
#define FB(N, B)                                                               \
	new TestFunc<fixed<B, overflow>, ::N, fixed<B, overflow>::N>(#N, _mutex)

#define FUNC(N)                                                                \
	FB(N, 0), FB(N, 1), FB(N, 2), FB(N, 3), FB(N, 4), FB(N, 5), FB(N, 6),      \
		FB(N, 7), FB(N, 8), FB(N, 9), FB(N, 10), FB(N, 11), FB(N, 12),         \
		FB(N, 13), FB(N, 14), FB(N, 15), FB(N, 16), FB(N, 17), FB(N, 18),      \
		FB(N, 19), FB(N, 20), FB(N, 21), FB(N, 22), FB(N, 23), FB(N, 24),      \
		FB(N, 25), FB(N, 26), FB(N, 27), FB(N, 28), FB(N, 29), FB(N, 30)

// Binary functions
#define FB2(N, B)                                                              \
	new TestFunc2<fixed<B, overflow>, ::N, fixed<B, overflow>::N>(#N, _mutex)

#define FUNC2(N)                                                               \
	FB2(N, 0), FB2(N, 1), FB2(N, 2), FB2(N, 3), FB2(N, 4), FB2(N, 5),          \
		FB2(N, 6), FB2(N, 7), FB2(N, 8), FB2(N, 9), FB2(N, 10), FB2(N, 11),    \
		FB2(N, 12), FB2(N, 13), FB2(N, 14), FB2(N, 15), FB2(N, 16),            \
		FB2(N, 17), FB2(N, 18), FB2(N, 19), FB2(N, 20), FB2(N, 21),            \
		FB2(N, 22), FB2(N, 23), FB2(N, 24), FB2(N, 25), FB2(N, 26),            \
		FB2(N, 27), FB2(N, 28), FB2(N, 29), FB2(N, 30)

// Boolean operators
#define FBB(N, B)                                                              \
	new TestFuncB<fixed<B, overflow>, ::N, fixed<B, overflow>::N>(#N, _mutex)

#define FUNCB(N)                                                               \
	FBB(N, 0), FBB(N, 1), FBB(N, 2), FBB(N, 3), FBB(N, 4), FBB(N, 5),          \
		FBB(N, 6), FBB(N, 7), FBB(N, 8), FBB(N, 9), FBB(N, 10), FBB(N, 11),    \
		FBB(N, 12), FBB(N, 13), FBB(N, 14), FBB(N, 15), FBB(N, 16),            \
		FBB(N, 17), FBB(N, 18), FBB(N, 19), FBB(N, 20), FBB(N, 21),            \
		FBB(N, 22), FBB(N, 23), FBB(N, 24), FBB(N, 25), FBB(N, 26),            \
		FBB(N, 27), FBB(N, 28), FBB(N, 29), FBB(N, 30)

deque<Test*> _tests{
	FUNC(fabs),		FUNC(floor),  FUNC(ceil),  FUNC(trunc),  FUNC(sqrt),
	FUNC(sin),		FUNC(cos),	FUNC(tan),   FUNC(exp),	FUNC(negate),
	FUNC2(fmod),	FUNC2(atan2), FUNC2(plus), FUNC2(minus), FUNC2(times),
	FUNC2(divide),  FUNCB(equal), FUNCB(neq),  FUNCB(lower), FUNCB(leq),
	FUNCB(greater), FUNCB(geq),
};

const int _num_tests = _tests.size();

// -----------------------------------------------------------------------------
// And finally...

void usage(const char* exe)
{
	fprintf(stderr, "Usage: %s [step]\n\n", exe);
	fprintf(stderr, "Test all math.h functions with a range of inputs.\n");
	fprintf(stderr, "Use step 1 for an exhaustive test. Default is 8191.\n");
}

int main(int argc, const char* argv[])
{
	if (argc > 2) {
		usage(argv[0]);
		return 1;
	}

	long step = 8191;
	if (argc == 2) {
		char* end;
		step = strtol(argv[1], &end, 10);
		if (*end) {
			fprintf(stderr, "** Expected a number but found: '%s'\n\n", argv[1]);
			usage(argv[0]);
			return 1;
		}
	}

	vector<unique_ptr<thread>> threads;
	for (int i = 0; i < 8; ++i) {
		unique_ptr<thread> t(new thread(run_tests_worker, step));
		threads.push_back(move(t));
	}

	bool failed = false;
	{
		unique_lock<mutex> lock(_mutex);
		while (!_failed && _finished < _num_tests) _cond.wait(lock);
		failed = _failed;
	}

	if (failed) {
		for (auto& t : threads) t->detach();
	}
	else
	{
		for (auto& t : threads) t->join();
	}

	unique_lock<mutex> lock(_mutex);
	printf("\n*** %s ***\n", _failed ? "FAILED" : "PASSED");
	return _failed ? 1 : 0;
}
