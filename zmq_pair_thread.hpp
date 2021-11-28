#pragma once

#include <iostream>
#include <atomic>
#include <memory>
#include <thread>

template <const char* logTag>
class ZmqPairThread {
public:
    ZmqPairThread() :
		m_running(true)
    {}
    virtual ~ZmqPairThread() {
		stop();
	}
	void log(const std::string& msg) const {
		std::cerr << "[log|" << logTag << "] " << msg << '\n';
	}
	void start() {
		log("starting client...");
		m_thread = std::make_unique<std::thread>(&ZmqPairThread::run, this);
	}
	void stop() {
		log("stopping client...");
		m_running.store(false);
		if (m_thread) {
			m_thread->join();
			m_thread.reset();
		}
	}

protected:
	virtual void run() const = 0;
	std::atomic_bool m_running;
	std::unique_ptr<std::thread> m_thread;
};
