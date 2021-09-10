#include "SharedConfig.hpp"
#include <argparse/argparse.hpp>
#include <drogon/drogon.h>
#include <functional>
#include <filesystem>

size_t Config::port = 0;
std::string Config::logFilePath = "";

int main(int argc, const char* argv[]) {
	argparse::ArgumentParser program("rpi-fan-serve");
	program.add_argument("<port>")
		.help("The port to listen on");
	program.add_argument("<log_file>")
		.help("The base log file to serve");
	try {
		program.parse_args(argc, argv);
	}
	catch (const std::runtime_error& err) {
		std::cout << err.what() << std::endl;
		std::cout << program;
		return 1;
	}
	auto portStr = program.get<std::string>("<port>");
	auto logFilePath = program.get<std::string>("<log_file>");

	size_t port = 0;
	try {
		port = std::stoul(portStr, nullptr, 10);
	} catch (std::invalid_argument) {
		std::cerr << "[error] invalid port: `" << portStr << "`\n";
		return 1;
	} catch (std::out_of_range) {
		std::cerr << "[error] out of range port: `" << portStr << "`\n";
		return 1;
	}
	if (port < 1'000 || port > 100'000) {
		std::cerr << "[error] port should be in range [1'000;100'000] `" << portStr << "`\n";
		return 1;
	}
	if (!std::filesystem::exists(logFilePath)) {
		std::cerr << "[error] base log file: `" << logFilePath << "` doesn't exist.";
		return 1;
	}
	Config::port = port;
	Config::logFilePath = logFilePath;

	drogon::app().addListener("0.0.0.0", Config::port).run();

    return 0;
}
