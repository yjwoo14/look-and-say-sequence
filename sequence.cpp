#include <iostream>
#include <string>
#include <functional>
#include <vector>

constexpr char terminate = 0;

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
		if (next != terminate) return;
		dest(terminate);
	}

	void read() {
		if (count == 0) return;
		for (const auto & c : std::to_string(count))
			dest(c);
		dest(last);
	}
};

struct SequencePrinter {
	void push(char next) {
		if (next == terminate) std::cout << std::endl;
		else std::cout << next;
	}
};

class Solver {
public:
	Solver() = delete;
	Solver(size_t N) : readers(N-1) {
		if (N < 1) throw std::runtime_error("N must be greater than 0.");
		
		using std::placeholders::_1;
		if (N == 1) {
			start = std::bind(&SequencePrinter::push, printer, _1);
			return;
		}

		start = std::bind(&SequenceReader::push, &readers[0], _1);
		for (size_t i = 0 ; i + 2 < N ; ++i)
			readers[i].dest = std::bind(&SequenceReader::push, &readers[i+1], _1);
		readers.back().dest = std::bind(&SequencePrinter::push, printer, _1);
	}

	void go() {
		start('1');
		start(terminate); 
	}

private:
	std::function<void(char)> start;
	std::vector<SequenceReader> readers;
	SequencePrinter printer;
};

int main(int argc, const char *argv[])
{
	size_t N;
	std::cin >> N;
	Solver solver(N);
	solver.go();
	return 0;
}
