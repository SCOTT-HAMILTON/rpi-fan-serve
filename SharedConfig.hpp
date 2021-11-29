#pragma once

#include <string>

struct SharedConfig {
	static size_t port;
	static size_t maxjobs;
	static std::string logFilePath;
};
