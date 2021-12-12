#pragma once

#include <iostream>
#include <zmq.hpp>
#include "../utils.hpp"
#include "../zmq_constants.hpp"
#include "../socket_locker.hpp"
#include "zmq_pair_qthread.hpp"

namespace ZmqDbusClientData {
	static const char TAG[] = "ZMQ-DBUS-client";
}

class ZmqDbusClient : public ZmqPairQThread {
	Q_OBJECT
public:
    ZmqDbusClient(QObject *parent = nullptr) :
		ZmqPairQThread(ZmqDbusClientData::TAG, parent)
    {}

protected:
	virtual void trySendCallback(zmq::socket_t& socket) = 0;
	virtual void receiveCallback(const std::string& msg) = 0;
	void run() override {
		zmq::context_t ctx;
		zmq::socket_t sock(ctx, zmq::socket_type::pair);
		std::cerr << "[debug] connecting...\n";
		try {
			sock.connect(std::string("ipc://") + SOCKET_FILE);
			std::cerr << "[debug] connect URL: "
					  << "ipc://" << SOCKET_FILE << '\n';
			sock.set(zmq::sockopt::linger, 0);
			std::cerr << "[debug] connected.\n";
			zmq::message_t msg;
			sock.send(zmq::str_buffer("hello world from client!"), zmq::send_flags::dontwait);
			sock.send(zmq::str_buffer("hello world from client!"), zmq::send_flags::dontwait);
		} catch (const zmq::error_t& e) {
			std::cerr << "[log|zmq-dbus-client] " << e.what() << "`\n";
		}
		while (m_running) {
			zmq::message_t msg(1024);
			try {
				zmq::recv_result_t r = sock.recv(msg, zmq::recv_flags::dontwait);
				if (r) {
					std::cerr << "[log|ZMQ-DBUS-client] received: `" << msg << "`\n";
					receiveCallback(msg.to_string());
				} else {
					trySendCallback(sock);
					sleep_seconds(1);
				}
			} catch (const zmq::error_t& e) {
				std::cerr << "[log|zmq-dbus-client] " << e.what() << "`\n";
			}
		}
		try {
			sock.close();
			ctx.close();
			ctx.shutdown();
		}  catch (const zmq::error_t& e) {
			std::cerr << "[log|zmq-dbus-client] " << e.what() << "`\n";
		}
		log("client stopped.");
	}
};
