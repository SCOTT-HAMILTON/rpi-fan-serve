#pragma once

#include <iostream>
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
				std::cerr << "[log|ZMQ-DBUS-server] received: `" << msg << "`\n";
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
