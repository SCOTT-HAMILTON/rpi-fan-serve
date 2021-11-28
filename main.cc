#include "SharedConfig.hpp"
#include "dbus_server.hpp"
#include "utils.hpp"
#include "zmq_dbus_client.hpp"
#include "zmq_dbus_server.hpp"
#include <argparse/argparse.hpp>
#include <drogon/drogon.h>
#include <functional>
#include <filesystem>
#include <time.h>

size_t Config::port = 0;
std::string Config::logFilePath = "";
size_t Config::maxjobs = 0;
std::atomic_bool dbus_running(true);

static void DBusTERMFunction(int sig)
{
    if (sig == SIGTERM)
    {
		std::cerr << "[log] SIGTERM signal received.\n";
		dbus_running.store(false);
    }
    else if (sig == SIGINT)
    {
		std::cerr << "[log] SIGINT signal received.\n";
		dbus_running.store(false);
    }
}

int main(int argc, const char* argv[]) {
	argparse::ArgumentParser program("rpi-fan-serve");
	program.add_argument("-p", "--port")
		.help("The port to listen on");
	program.add_argument("-l", "--log-file")
		.help("The base log file to serve");
	program.add_argument("-j", "--max-jobs")
		.help("The max number of threads to use");
	program.add_argument("--dbus")
	  .default_value(false)
	  .implicit_value(true)
	  .help("Run the dbus server instead of the rpi-fan-server");
	try {
		program.parse_args(argc, argv);
	}
	catch (const std::runtime_error& err) {
		std::cout << err.what() << std::endl;
		std::cout << program;
		return 1;
	}
	bool isDbus = (program["--dbus"] == true);
	std::string portStr("");
	std::string logFilePath("");
	std::string maxJobsStr("");
	{
		auto fn = program.present("-p");
		if (!isDbus && fn) {
			portStr = *fn;
		}
	}
	{
		auto fn = program.present("-l");
		if (!isDbus && fn) {
			logFilePath = *fn;
		}
	}
	{
		auto fn = program.present("-j");
		if (!isDbus && fn) {
			maxJobsStr = *fn;
		}
	}
	if (!isDbus && (
		portStr == "" ||
		logFilePath == "" ||
		maxJobsStr == "")) {
		std::cout << "port, log base file and max jobs are required except for dbus mode" << std::endl;
		std::cout << program;
		return 1;
	}
	
	if (!isDbus) {
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
		Config::port = port;
		Config::logFilePath = logFilePath;
		Config::maxjobs = maxjobs;
		ZmqDbusServer zmqDbusServer;
		zmqDbusServer.start();
		drogon::app().addListener("0.0.0.0", Config::port).run();
		zmqDbusServer.stop();
	} else {
		{
			struct sigaction sa;
			sa.sa_handler = DBusTERMFunction;
			sigemptyset(&sa.sa_mask);
			if (sigaction(SIGINT, &sa, NULL) == -1)
			{
				std::cerr << "[error] sigaction() failed, can't set SIGINT handler.\n";
				return 1;
			}
			if (sigaction(SIGTERM, &sa, NULL) == -1)
			{
				std::cerr << "[error] sigaction() failed, can't set SIGTERM handler.\n";
				return 1;
			}
		}
		DbusServer dbusServer;
		ZmqDbusClient zmqDbusClient;
		dbusServer.start();
		zmqDbusClient.start();
		while (dbus_running.load()) {
			sleep_seconds(1);
		}
		dbusServer.stop();
		zmqDbusClient.stop();
	}

    return 0;
}
