#pragma once

#include <iostream>
#include <variant>
#include <exception>
#include <string>
#include <zmq.hpp>
#include "zmq_constants.hpp"
#include "utils.hpp"
#include "zmq_pair_thread.hpp"

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
	ParseMessageResult parse_message(const std::string& msg) {
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
		sock.bind("tcp://127.0.0.1:" + std::to_string(ZmqConstants::DBUS_PORT));
		zmq::message_t msg;
		while (m_running) {
			zmq::message_t msg(1024);
			zmq::recv_result_t r = sock.recv(msg, zmq::recv_flags::dontwait);
			if (r) {
				receiveCallback(msg.to_string());
				sock.send(zmq::str_buffer("hello world from server!"), zmq::send_flags::dontwait);
			} else {
				trySendCallback(sock);
				sleep_seconds(1);
			}
		}
		log("server stopped.");
	}
};
