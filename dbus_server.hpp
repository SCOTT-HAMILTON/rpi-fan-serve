#pragma once

#include <org/scotthamilton/RpiFanServe/error.hpp>
#include <org/scotthamilton/RpiFanServe/server.hpp>
#include <sdbusplus/server.hpp>

#include <iostream>
#include <string_view>
#include <atomic>
#include <memory>
#include <thread>
#include <tbb/concurrent_queue.h>
#include "zmq_dbus_client.hpp"

using RpiFanServe_inherit =
    sdbusplus::server::object_t<sdbusplus::org::scotthamilton::server::RpiFanServe>;

struct RpiFanServe : RpiFanServe_inherit
{
	int64_t m_cacheLifeExpectancy = 60*30;
	std::function<void(int)> m_newCacheLifeExpectancyCb;
    /** Constructor */
    RpiFanServe(
			sdbusplus::bus_t& bus,
			const char* path,
			const std::function<void(int)> callback) :
        RpiFanServe_inherit(bus, path),
		m_newCacheLifeExpectancyCb(callback)
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
		m_newCacheLifeExpectancyCb(m_cacheLifeExpectancy);
        return m_cacheLifeExpectancy;
    }
};

class DbusServer {
	class CtrlDbusClient: public ZmqDbusClient {
	public:
		CtrlDbusClient(DbusServer& parent) : m_parent(parent) {}
		void queue_send(const std::string& msg) {
			m_messages.push(msg);
		}
	protected:
		void receiveCallback(const std::string& msg) override {
			std::cerr << "[debug] lol received " << msg << '\n';
		}
		void trySendCallback(zmq::socket_t& socket) override {
			while (m_messages.size() > 0) {
				std::string msg;
				if (m_messages.try_pop(msg)) {
					socket.send(zmq::buffer(msg), zmq::send_flags::dontwait);
				}
			}
		}
		DbusServer& m_parent;
	private:
		tbb::concurrent_bounded_queue<std::string> m_messages;
	};
public:
    DbusServer() :
		m_running(true),
		m_ctrlDbusClient(*this)
    {
		m_ctrlDbusClient.start();
	}
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
	std::unique_ptr<RpiFanServe> m_rpiFanServeObject;
	CtrlDbusClient m_ctrlDbusClient;
	void run() {
		// Define a dbus path location to place the object.
		constexpr auto path = "/org/scotthamilton/rpifanserver";

		// Create a new bus and affix an object manager for the subtree path we
		// intend to place objects at..
		auto b = sdbusplus::bus::new_default();
		sdbusplus::server::manager_t m{b, path};

		// Reserve the dbus service name : org.scotthamilton.RpiFanServe
		b.request_name("org.scotthamilton.RpiFanServe");

		// Create object at /org/scotthamilton/rpifanserver
		m_rpiFanServeObject = std::make_unique<RpiFanServe>(b, path,
				[&](int newValue){
			m_ctrlDbusClient.queue_send(
				ZmqConstants::NEW_CACHE_LIFE_EXPECTANCY_KEY
					+ std::string(":")
					+ std::to_string(newValue)
			);
		});
		std::cerr << "[log|DBUS-server] server started.\n";
		while (m_running.load())
		{
			b.process_discard(); // discard any unhandled messages
			b.wait(1'000'000); // 1 second of timeout is fine.
		}
		std::cerr << "[log|DBUS-server] stopping stopped.\n";
	}
};
