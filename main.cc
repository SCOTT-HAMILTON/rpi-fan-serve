#include "SharedConfig.hpp"
#include "utils.hpp"
#include "socket_locker.hpp"
#include <argparse/argparse.hpp>
#include <drogon/drogon.h>
#include <functional>
#include <filesystem>
#include <time.h>

size_t SharedConfig::port = 0;
std::string SharedConfig::logFilePath = "";
size_t SharedConfig::maxjobs = 0;

int main(int argc, const char* argv[]) {
	argparse::ArgumentParser program("rpi-fan-serve");
	program.add_argument("-p", "--port")
		.help("The port to listen on");
	program.add_argument("-l", "--log-file")
		.help("The base log file to serve");
	program.add_argument("-j", "--max-jobs")
		.help("The max number of threads to use");
	try {
		program.parse_args(argc, argv);
	}
	catch (const std::runtime_error& err) {
		std::cout << err.what() << std::endl;
		std::cout << program;
		return 1;
	}
	std::string portStr("");
	std::string logFilePath("");
	std::string maxJobsStr("");
	{
		auto fn = program.present("-p");
		if (fn) {
			portStr = *fn;
		}
	}
	{
		auto fn = program.present("-l");
		if (fn) {
			logFilePath = *fn;
		}
	}
	{
		auto fn = program.present("-j");
		if (fn) {
			maxJobsStr = *fn;
		}
	}
	if (portStr == "" ||
		logFilePath == "" ||
		maxJobsStr == "") {
		std::cout << "port, log base file and max jobs are required" << std::endl;
		std::cout << program;
		return 1;
	}
	
	size_t port = 0;
	try {
		port = std::stoul(portStr, nullptr, 10);
	} catch (std::invalid_argument&) {
		std::cerr << "[error] invalid port: `" << portStr << "`\n";
		return 1;
	} catch (std::out_of_range&) {
		std::cerr << "[error] out of range port: `" << portStr << "`\n";
		return 1;
	}
	if (port < 1'000 || port > 100'000) {
		std::cerr << "[error] port should be in range [1'000;100'000] `" << portStr << "`\n";
		return 1;
	}

	SocketLocker<LOCK_FILE> socketLocker;
	if (!socketLocker.checkAndTryLock()) {
		return 1;
	}

	if (!std::filesystem::exists(logFilePath)) {
		std::cerr << "[error] base log file: `" << logFilePath << "` doesn't exist.";
		return 1;
	}

	size_t maxjobs = 0;
	try {
		maxjobs = std::stoul(maxJobsStr, nullptr, 10);
	} catch (std::invalid_argument&) {
		std::cerr << "[error] invalid max jobs: `" << maxJobsStr << "`\n";
		return 1;
	} catch (std::out_of_range&) {
		std::cerr << "[error] out of range max jobs: `" << maxJobsStr << "`\n";
		return 1;
	}
	if (maxjobs > 10'000) {
		std::cerr << "[error] max jobs should be in range [0;10'000] `" << maxJobsStr << "`\n";
		return 1;
	}
	SharedConfig::port = port;
	SharedConfig::logFilePath = logFilePath;
	SharedConfig::maxjobs = maxjobs;
	drogon::app().addListener("0.0.0.0", SharedConfig::port).run();

    return 0;
}
