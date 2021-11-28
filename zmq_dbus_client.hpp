#pragma once

#include <iostream>
#include <zmq.hpp>
#include "zmq_constants.hpp"
#include "utils.hpp"
#include "zmq_pair_thread.hpp"

namespace ZmqDbusClientData {
	static const char TAG[] = "ZMQ-DBUS-client";
}

class ZmqDbusClient : public ZmqPairThread<ZmqDbusClientData::TAG> {
public:
    ZmqDbusClient() :
		ZmqPairThread<ZmqDbusClientData::TAG>()
    {}

protected:
	void run() const {
		zmq::context_t ctx;
		zmq::socket_t sock(ctx, zmq::socket_type::pair);
		sock.connect("tcp://localhost:" + std::to_string(ZmqConstants::PORT));
		zmq::message_t msg;
		sock.send(zmq::str_buffer("hello world from client!"), zmq::send_flags::dontwait);
		while (m_running) {
			zmq::message_t msg(1024);
			zmq::recv_result_t r = sock.recv(msg, zmq::recv_flags::dontwait);
			if (r) {
				std::cerr << "[log|ZMQ-DBUS-client] received: `" << msg << "`\n";
			} else {
				sleep_seconds(1);
			}
		}
		log("client stopped.");
	}
};
