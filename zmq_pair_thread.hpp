#pragma once

#include <iostream>
#include <atomic>
#include <memory>
#include <thread>
#include <string>

template <const char* logTag>
class ZmqPairThread {
public:
    ZmqPairThread() :
		m_running(true)
    {}
    virtual ~ZmqPairThread() {
		stop();
	}
	void log(std::string_view msg) const noexcept {
		std::cerr << "[log|" << logTag << "] " << msg << '\n';
	}
	void start() noexcept {
		log("starting client...");
		m_thread = std::make_unique<std::thread>(&ZmqPairThread::run, this);
	}
	void stop() noexcept {
		log("stopping client...");
		m_running.store(false);
		if (m_thread) {
			if (m_thread->joinable()) {
				m_thread->join();
			}
			m_thread.reset();
		}
	}

protected:
	virtual void run() = 0;
	std::atomic_bool m_running;
	std::unique_ptr<std::thread> m_thread;
};
