#pragma once

#include <time.h>

static inline void sleep_seconds(int seconds) {
	auto t = timespec{};
	t.tv_sec = seconds;
	t.tv_nsec = 0;
	nanosleep(&t, NULL);
}
