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
	t.tv_sec = ms / 1000;
	t.tv_nsec = (ms % 1000) * 1000000L;
	nanosleep(&t, NULL);
}
