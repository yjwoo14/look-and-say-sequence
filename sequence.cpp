#include <iostream>
#include <string>
#include <functional>
#include <vector>
#include <sstream>
#include <algorithm>
#include <limits>

constexpr char terminal = 0;

/* Reader
   Reader reads a sequence and push the characters generated
   from the reading. Reader receives each character at a time, and whenever
   it can say a part of the sequence, it reads the part. 
*/
struct Reader {
	char last = 0;
	size_t count = 0;
	std::function<void(char)> dest;

	void push(char next) {
		if (last == next) {
			count++;
			return;
		}
		read();
		last = next;
		count = 1;
		if (next != terminal) return;
		dest(terminal);
	}

	void read() {
		if (count == 0) return;
		if (count < 10) {
			dest((char)(count + '0'));
		} else {
			// Does this case ever happen?
			for (const auto & c : std::to_string(count))
				dest(c);
		}
		dest(last);
	}
};

/* Printer
   Simply print pushed item */
struct Printer {
	std::function<void()> terminate;

	void push(char next) {
		if (next == terminal) std::cout << std::endl;
		else std::cout << next;
	}
};

/* Picker
   print the characters at the given indices (1 base) */ 
struct Picker {
	std::vector<size_t> M;
	size_t idx = 0;
	std::function<void()> terminate;

	Picker() = delete;
	Picker(const std::vector<size_t> & M) 
		: M(M) {
		std::sort(this->M.begin(), this->M.end(), std::greater<size_t>());
	}

	void push(char next) {
		if (M.empty() || next == terminal)
			finish();
		++idx;
		if (idx != M.back()) return;
		std::cout << M.back() << ": " << next << std::endl;
		M.pop_back();
	}

	void finish() {
		if (!M.empty()) {
			std::reverse(M.begin(), M.end());
			std::cout << "The number is too short to find [ ";
			for (const auto i : M) std::cout << i << " ";
			std::cout << "] th character(s)." << std::endl;
		}
		terminate();
	}
};

/* LengthPrinter
   print length of the sequence */
struct LengthPrinter {
	size_t length = 0;
	std::function<void()> terminate;
	void push(char next) { 
		if (next == terminal) {
			std::cout << "Length: " << length << std::endl;
			return;
		}
		length++;
		if (length == std::numeric_limits<size_t>::max()) 
			throw std::runtime_error("Cannot represent the length of the sequence in size_t type");
	}
};

/* First version of solving look and say sequence 
   As it uses call stack, N is limited by stack size */
template <typename End>
class Solver1 {
public:
	Solver1() = delete;
	Solver1(size_t N, End end = End()) : readers(N-1), end(end) {
		if (N < 1) throw std::runtime_error("N must be greater than 0.");
		
		this->end.terminate = std::bind(&Solver1::terminate, this);
		if (N == 1) {
			start = std::bind(&End::push, &this->end, std::placeholders::_1);
			return;
		}

		start = std::bind(&Reader::push, &readers[0], std::placeholders::_1);
		for (size_t i = 0 ; i + 1 < readers.size() ; ++i)
			readers[i].dest = std::bind(&Reader::push, &readers[i+1], std::placeholders::_1);
		readers.back().dest = std::bind(&End::push, &this->end, std::placeholders::_1);
	}

	void go() {
		start('1');
		start(terminal); 
	}

	void terminate() {
		auto empty = [](char){};
		for (size_t i = 0 ; i < readers.size() ; ++i)
			readers[i].dest = std::bind(empty, std::placeholders::_1);
	}

private:
	std::function<void(char)> start;
	std::vector<Reader> readers;
	End end;
};

/* Second version of solving look and say sequence
   It stores events in heap to get round stack problem in version 1 */
template <typename End>
class Solver2 {
public:
	struct Event {
		size_t target;
		char item;
	};

	class EventContainer {
	private:
		std::vector<Event> events;
	public:
		void push(size_t target, char item) {
			events.push_back(Event{target, item});
			// reorder
			auto bit = events.rbegin();
			for (auto it = std::next(events.rbegin()); it != events.rend(); ++it) {
				if (it->target != target) break;
				std::swap(*bit, *it);
				bit = it;
			}
		}
		Event pop() {
			Event ret = events.back();
			events.pop_back();
			return ret;
		}
		bool empty() const {
			return events.empty();
		}
		void clear() {
			events.clear();
		}
		size_t capacity() const {
			return events.capacity();
		}
	};

	Solver2() = delete;
	Solver2(size_t N, End end = End()) : readers(N-1), end(end) {
		if (N < 1) throw std::runtime_error("N must be greater than 0.");

		this->end.terminate = std::bind(&Solver2::terminate, this);
		auto pushEvent = [this](size_t target, char item) {
			events.push(target, item);
		};

		for (size_t i = 0 ; i + 1 < N ; ++i)
			readers[i].dest = std::bind(pushEvent, i+1, std::placeholders::_1);
	}

	void go() {
		events.push(0, '1');
		events.push(0, terminal);
		while(!events.empty()) {
			const auto event = events.pop();
			if (event.target < readers.size())
				readers[event.target].push(event.item);
			else
				end.push(event.item);
		}
	}

	void terminate() {
		events.clear();
		std::cerr << "Event container capacity: " << events.capacity()
		          << " items (" << events.capacity() * sizeof(Event)
		          << " bytes)." << std::endl;
	}

private:
	std::vector<Reader> readers;
	End end;
	EventContainer events;
};

std::pair<size_t, std::vector<size_t>> getInput() {
	std::cout << "Input format: N         - to print the N'th complete sequence." << std::endl;
	std::cout << "              N [Mi...] - to print the Mi'th characters of the N'th sequence." << std::endl;
	std::cout << "              0 N       - to print the length of the N'th sequence." << std::endl;
	std::string input;
	std::getline(std::cin, input);
	input.erase(input.find_last_not_of(" \n\r\t")+1);

	size_t N=0;
	std::vector<size_t> M(0);
	std::stringstream ss(input);
	ss >> N;
	while(!ss.eof()) {
		size_t m;
		ss >> m;
		M.push_back(m);
	}
	return {N, M};
}

#define solve(Solver) \
if (N != 0 && !M.empty()) { \
	Solver<Picker> solver(N, Picker(M)); \
	solver.go(); \
} else if (N != 0) { \
	Solver<Printer> solver(N); \
	solver.go(); \
} else if (!M.empty()) { \
	Solver<LengthPrinter> solver(M.front()); \
	solver.go(); \
}

int main(int argc, const char *argv[])
{
	size_t N;
	std::vector<size_t> M;
	std::tie(N, M) = getInput();
	
	if (N < 10000) {
		// Solver1 cannot solve large N becase of stack size
		std::cout << std::endl;
		std::cout << "<Version 1>" << std::endl;
		solve(Solver1);
	}

	std::cout << std::endl;
	std::cout << "<Version 2>" << std::endl;
	solve(Solver2);
	return 0;
}
