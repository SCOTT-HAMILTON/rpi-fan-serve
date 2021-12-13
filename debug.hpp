#pragma once

#include <chrono>
#include <iostream>

using namespace std::chrono;

template<const char* funcName>
class DebugChrono {
public:
	DebugChrono() : 
		start(high_resolution_clock::now())
	{}
	~DebugChrono() {
		auto end = high_resolution_clock::now();
		std::cerr << "[debug] " << funcName << " "
				  << duration_cast<microseconds>(end-start).count()
				  << "µs.\n";
	}
private:
	time_point<high_resolution_clock> start;
};
