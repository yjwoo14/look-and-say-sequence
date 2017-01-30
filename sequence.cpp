#include <iostream>
#include <string>
#include <functional>
#include <vector>
#include <sstream>

constexpr char terminal = 0;

struct SequenceReader {
	char last;
	size_t count;
	std::function<void(char)> dest;

	SequenceReader(): last(0), count(0) {}
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

struct SequencePrinter {
	std::function<void()> terminate;

	void push(char next) {
		if (next == terminal) std::cout << std::endl;
		else std::cout << next;
	}
};

struct Picker {
	size_t M;
	std::function<void()> terminate;

	Picker() = delete;
	Picker(size_t M) : M(M) {}

	void push(char next) {
		if (M == 0) return;
		--M;
		if (M != 0) return;
		std::cout << next << std::endl;
		terminate();
	}
};

/* First version of solving look and say sequence */
/* As it uses call stack, N is limited by stack size */
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

		start = std::bind(&SequenceReader::push, &readers[0], std::placeholders::_1);
		for (size_t i = 0 ; i + 1 < readers.size() ; ++i)
			readers[i].dest = std::bind(&SequenceReader::push, &readers[i+1], std::placeholders::_1);
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
	std::vector<SequenceReader> readers;
	End end;
};

/* Second version of solving look and say sequence */
/* It stores events in heap to get round stack problem in version 1 */
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
			for (auto it = events.rbegin(); it != events.rend(); ++it) {
				if (it == events.rbegin()) continue;
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
	}

private:
	std::vector<SequenceReader> readers;
	End end;
	EventContainer events;
};

std::pair<size_t, size_t> getInput() {
	std::cout << "Input format: N   - to obtain N'th complete sequence." << std::endl;
	std::cout << "              N M - to obtain M'th character of N'th sequence." << std::endl;
	std::string input;
	std::getline(std::cin, input);
	size_t N=0, M=0;
	std::stringstream ss(input);
	if (input.find(" ") != std::string::npos) {
		ss >> N >> M;
	} else {
		ss >> N;
	}
	return {N, M};
}

int main(int argc, const char *argv[])
{
	size_t N, M;
	std::tie(N, M) = getInput();

	if (N < 10000) {
		// Solver1 cannot solve large N becase of stack size
		std::cout << "Version 1: ";
		if (M != 0) {
			Solver1<Picker> solver(N, Picker(M));
			solver.go();
		} else {
			Solver1<SequencePrinter> solver(N);
			solver.go();
		}
	}

	std::cout << "Version 2: ";
	if (M != 0) {
		Solver2<Picker> solver(N, Picker(M));
		solver.go();
	} else {
		Solver2<SequencePrinter> solver(N);
		solver.go();
	}
	return 0;
}
