#pragma once

#include <org/scotthamilton/RpiFanServe/error.hpp>
#include <org/scotthamilton/RpiFanServe/server.hpp>
#include <sdbusplus/server.hpp>

#include <iostream>
#include <string_view>
#include <atomic>
#include <memory>
#include <thread>

using RpiFanServe_inherit =
    sdbusplus::server::object_t<sdbusplus::org::scotthamilton::server::RpiFanServe>;

struct RpiFanServe : RpiFanServe_inherit
{
	int64_t m_cacheLifeExpectancy = 60*30;
    /** Constructor */
    RpiFanServe(sdbusplus::bus_t& bus, const char* path) :
        RpiFanServe_inherit(bus, path)
    {}

	int64_t cacheLifeExpectancy() const override
    {
		return m_cacheLifeExpectancy;
	}
	int64_t cacheLifeExpectancy(
			int64_t value,
			bool skipSignal) override
	{
		return cacheLifeExpectancy(value);
	}
	int64_t cacheLifeExpectancy(int64_t value) override
	{
        using sdbusplus::org::scotthamilton::RpiFanServe::Error::InvalidCacheLifeExpectancy;
        if (value <= 60)
        {
            throw InvalidCacheLifeExpectancy();
        }
		std::cerr << "[log] cache life expectancy updated " << m_cacheLifeExpectancy 
				  << " -> " << value << '\n';
		m_cacheLifeExpectancy = value;
        return m_cacheLifeExpectancy;
    }
};

class DbusServer {
public:
    DbusServer() :
		m_running(true)
    {}
	void start() {
		// Handle dbus processing forever.
		std::cerr << "[log|DBUS-server] starting server...\n";
		m_thread = std::make_unique<std::thread>(&DbusServer::run, this);
	}
	void stop() {
		std::cerr << "[log|DBUS-server] stopping server...\n";
		m_running.store(false);
		m_thread->join();
	}

private:
	std::atomic_bool m_running;
	std::unique_ptr<std::thread> m_thread;
	void run() const {
		// Define a dbus path location to place the object.
		constexpr auto path = "/org/scotthamilton/rpifanserver";

		// Create a new bus and affix an object manager for the subtree path we
		// intend to place objects at..
		auto b = sdbusplus::bus::new_default();
		sdbusplus::server::manager_t m{b, path};

		// Reserve the dbus service name : org.scotthamilton.RpiFanServe
		b.request_name("org.scotthamilton.RpiFanServe");

		// Create a calculator object at /org/scotthamilton/rpifanserver
		RpiFanServe c1{b, path};
		std::cerr << "[log|DBUS-server] server started.\n";
		while (m_running.load())
		{
			b.process_discard(); // discard any unhandled messages
			b.wait(1'000'000); // 1 second of timeout is fine.
		}
		std::cerr << "[log|DBUS-server] stopping stopped.\n";
	}
};
