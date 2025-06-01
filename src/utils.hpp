#ifndef IFMETRICA_UTILS_HPP
#define IFMETRICA_UTILS_HPP

#include <cstdint>
#include <chrono>
#include <thread>

using utime_t = uint64_t;

inline utime_t currentTimeUs() {
	using namespace std::chrono;
	return duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
}

inline void sleepUntilNextSec(utime_t offset = 1) {
	using namespace std::chrono;
	constexpr utime_t onesecUs = 1000000;
	utime_t currentUs = currentTimeUs() % onesecUs;
	std::this_thread::sleep_for(microseconds(onesecUs - currentUs + offset));
}

#endif //IFMETRICA_UTILS_HPP
