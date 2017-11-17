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

thread_local bool _overflow = false;

void overflow() { _overflow = true; }

bool get_overflow()
{
	bool result = _overflow;
	_overflow = false;
	return result;
}

// -----------------------------------------------------------------------------

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
		print("%13.6f: expected %13.6f, got %13.6f", val, expected, actual);
	}
};

// -----------------------------------------------------------------------------

#define FB(N, B)                                                               \
	new TestFunc<fixed<B, overflow>, ::N, fixed<B, overflow>::N>(#N, _mutex)

#define FUNC(N)                                                                \
	FB(N, 0), FB(N, 1), FB(N, 2), FB(N, 3), FB(N, 4), FB(N, 5), FB(N, 6),      \
		FB(N, 7), FB(N, 8), FB(N, 9), FB(N, 10), FB(N, 11), FB(N, 12),         \
		FB(N, 13), FB(N, 14), FB(N, 15), FB(N, 16), FB(N, 17), FB(N, 18),      \
		FB(N, 19), FB(N, 20), FB(N, 21), FB(N, 22), FB(N, 23), FB(N, 24),      \
		FB(N, 25), FB(N, 26), FB(N, 27), FB(N, 28), FB(N, 29), FB(N, 30)

mutex _mutex{};
condition_variable _cond{};
int _finished = 0;
bool _failed = false;

deque<Test*> _tests{
	FUNC(fabs), FUNC(floor), FUNC(ceil), FUNC(trunc),

	FUNC(sqrt), FUNC(sin),   FUNC(cos),  FUNC(tan),   FUNC(exp),
};

const int _num_tests = _tests.size();

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
