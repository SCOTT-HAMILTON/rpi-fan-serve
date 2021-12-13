#pragma once

#include <iostream>
#include <variant>
#include <exception>
#include <string>
#include <zmq.hpp>
#include "zmq_constants.hpp"
#include "utils.hpp"
#include "zmq_pair_thread.hpp"
#include "socket_locker.hpp"
#include "debug.hpp"

namespace ZmqDbusServerData {
	static const char TAG[] = "ZMQ-DBUS-server";
}

class ZmqDbusServer : public ZmqPairThread<ZmqDbusServerData::TAG> {
public:
    ZmqDbusServer() :
		ZmqPairThread<ZmqDbusServerData::TAG>()
    {}
	struct CacheLifeExpectancyMessage {
		int value;
	};
	enum None { NONE };
	using ParseMessageResult = std::variant<CacheLifeExpectancyMessage, None>;
	static ParseMessageResult parse_message(const std::string& msg)  noexcept {
		if (msg.substr(0, 5) == "NCLE:") {
			auto valueStr = msg.substr(5, msg.size()-5);
			try {
				CacheLifeExpectancyMessage r;
				r.value = std::stoi(valueStr);
				return ParseMessageResult(r);
			} catch (std::exception& e) {
				std::cerr << "[error] received invalid value"
						  << " for cache life expectancy: " << msg;
			}
		}
		return ParseMessageResult(NONE);
	}

protected:
	virtual void trySendCallback(zmq::socket_t& socket) = 0;
	virtual void receiveCallback(const std::string& msg) = 0;
	void run() {
		zmq::context_t ctx;
		zmq::socket_t sock(ctx, zmq::socket_type::pair);
		sock.bind(std::string("ipc://") + SOCKET_FILE);
		sock.set(zmq::sockopt::linger, 0);
		zmq::message_t msg;
		while (m_running) {
			zmq::message_t msg(1024);
			bool valid = false;
			{
				do {
					if (!m_running) {
						break;
					}
					zmq::recv_result_t r = sock.recv(msg, zmq::recv_flags::dontwait);
					if (r) {
						valid = true;
						receiveCallback(msg.to_string());
						std::cerr << msg.to_string() << '\n';
					} else {
						valid = false;
						trySendCallback(sock);
						sleep_seconds(1);
					}
				} while (valid);
			}
		}
		sock.close();
		ctx.close();
		ctx.shutdown();
		log("server stopped.");
	}
};
