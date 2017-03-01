#include <iostream>
#include <boost/coroutine/coroutine.hpp>
#include <boost/version.hpp>

// need compile option -lboost_system -lboost_coroutine --std=c++14

typedef boost::coroutines::symmetric_coroutine<void> coroutine;
typedef coroutine::call_type call_type;
typedef coroutine::yield_type yield_type;

constexpr char terminate = (char)255;

call_type ant(int n, char &ret) {
	if (n == 1) {
		return call_type([&ret](yield_type & yield) {
			ret = '1';
			yield();
			ret = terminate;
			yield();
		}, boost::coroutines::attributes(boost::coroutines::no_stack_unwind));
	}

	return call_type([&ret, n, prev='0', count='0'] (yield_type & yield) mutable {
		char next;
		auto gen = ant(n-1, next);
		auto yields = [&](){ ret = count; yield(); ret = prev ; yield(); };
		while (true) {
			gen();
			if (next == terminate) {
				yields();
				ret = terminate;
				yield();
				return;
			}
			if (next == prev) {
				count++;
				continue;
			}
			if (count != '0') 
				yields();
			prev = next;
			count = '1';
		}
	}, boost::coroutines::attributes(boost::coroutines::no_stack_unwind));
}

int main(int argc, const char *argv[])
{

	char ret;
	auto gen = ant(3000000, ret);
	while(true) {
		gen();
		if (ret == terminate) break;
		std::cout << ret;
	}
	std::cout << std::endl;

	return 0;
}
