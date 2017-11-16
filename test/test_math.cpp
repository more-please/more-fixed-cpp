#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>

#include "../include/more_fixed.h"

using namespace std;

// -----------------------------------------------------------------------------

thread_local bool _overflow = false;

void raise_overflow() { _overflow = true; }

bool get_overflow()
{
	bool result = _overflow;
	_overflow = false;
	return result;
}

// -----------------------------------------------------------------------------

struct Test
{
	virtual const char* name() const = 0;
	virtual int bits() const = 0;

	// Return true on success
	virtual bool test_all(int step) = 0;

	virtual ~Test() = default;
};

template <
	int BITS,
	double (*DFUNC)(double),
	more::fixed<BITS, raise_overflow> (*FFUNC)(more::fixed<BITS, raise_overflow>)>
struct FuncTest : public Test
{
	FuncTest(const char* name, mutex& mutex) : _name(name), _mutex(mutex) {}

	virtual const char* name() const { return _name; }
	virtual int bits() const { return BITS; }

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

		unique_lock<mutex> lock(_mutex);
		printf("%8s.%02d: %s\n", _name, BITS, ok ? "ok" : "FAILED");
		return ok;
	}

private:
	const char* _name;
	mutex& _mutex;

	typedef more::fixed<BITS, raise_overflow> fixed;

	bool test_repr(int32_t repr) { return test(fixed::from_repr(repr)); }

	bool test(fixed fval)
	{
		const double nan = numeric_limits<double>::quiet_NaN();
		const double dval = double(fval);
		const double exact = DFUNC(dval);

		int overflow = 0;
		get_overflow();

		fixed fexpected = fixed(exact);
		if (get_overflow()) ++overflow;

		fixed factual = FFUNC(fval);
		if (get_overflow()) ++overflow;

		int err = factual.repr() - fexpected.repr();

		if (overflow) {
			if (overflow != 2) {
				double expected = double(fixed(exact));
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
		unique_lock<mutex> lock(_mutex);
		printf(
			"%8s.%02d: %13.6f: expected %13.6f, got %13.6f\n",
			_name,
			BITS,
			val,
			expected,
			actual);
	}
};

// -----------------------------------------------------------------------------

#define FB(NAME, BITS)                                                         \
	new FuncTest<BITS, ::NAME, more::fixed<BITS, raise_overflow>::NAME>(       \
		#NAME, _mutex)

#define FUNC(NAME)                                                             \
	FB(NAME, 0), FB(NAME, 1), FB(NAME, 2), FB(NAME, 3), FB(NAME, 4),           \
		FB(NAME, 5), FB(NAME, 6), FB(NAME, 7), FB(NAME, 8), FB(NAME, 9),       \
		FB(NAME, 10), FB(NAME, 11), FB(NAME, 12), FB(NAME, 13), FB(NAME, 14),  \
		FB(NAME, 15), FB(NAME, 16), FB(NAME, 17), FB(NAME, 18), FB(NAME, 19),  \
		FB(NAME, 20), FB(NAME, 21), FB(NAME, 22), FB(NAME, 23), FB(NAME, 24),  \
		FB(NAME, 25), FB(NAME, 26), FB(NAME, 27), FB(NAME, 28), FB(NAME, 29),  \
		FB(NAME, 30)

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
