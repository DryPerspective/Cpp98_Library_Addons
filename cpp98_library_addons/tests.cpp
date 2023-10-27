#define DP_CPP20

#include <iostream>

#include "cpp98/value_ptr.h"

struct noisy {
	noisy() {
		std::cout << "DEFAULT CTOR\n";
	}
	noisy(const noisy&) {
		std::cout << "COPY CTOR\n";
	}
	noisy& operator=(const noisy&) {
		std::cout << "COPY ASSIGN\n";
	}
	~noisy() {
		std::cout << "DTOR\n";
	}
};

struct holds_noisy {
	dp::value_ptr<noisy> held;

	holds_noisy() = default;
	holds_noisy(noisy* in) : held(in) {}
};

int main() {

	holds_noisy hn{ new noisy{} };

	auto hn2{ hn };


	return 0;
}