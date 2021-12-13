#pragma once

#include <time.h>

static inline void sleep_seconds(int seconds) {
	auto t = timespec{};
	t.tv_sec = seconds;
	t.tv_nsec = 0;
	nanosleep(&t, NULL);
}


static inline void sleep_ms(int ms) {
	auto t = timespec{};
	t.tv_sec = 0;
	t.tv_nsec = ms*10^6;
	nanosleep(&t, NULL);
}
