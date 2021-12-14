#pragma once

#include <QObject>
#include <tbb/concurrent_queue.h>
#include "RpiFanServeObject.h"
#include "dbus_proxy.h"
#include "dbus_adaptor.h"
#include "zmq_dbus_client.hpp"
#include "../zmq_constants.hpp"

class DbusServer: public QObject
{
    Q_OBJECT
	class CtrlDbusClient: public ZmqDbusClient {
	public:
		CtrlDbusClient(DbusServer& parent) :
			ZmqDbusClient(),
			m_parent(parent) {}
		void queue_send(const std::string& msg) {
			m_messages.push(msg);
		}
	protected:
		void trySendCallback(zmq::socket_t& socket) override {
			while (m_messages.size() > 0) {
				std::string msg;
				if (m_messages.try_pop(msg)) {
					socket.send(zmq::str_buffer("A"), zmq::send_flags::sndmore|zmq::send_flags::dontwait);
					socket.send(zmq::buffer(msg), zmq::send_flags::dontwait);
				}
			}
		}
		DbusServer& m_parent;
	private:
		tbb::concurrent_bounded_queue<std::string> m_messages;
	};
public:
    DbusServer(QObject *parent = nullptr);
    ~DbusServer();

public slots:
	void onCacheLifeExpectancyChanged();	

private:
	CtrlDbusClient m_ctrlDbusClient;
	RpiFanServeObject* m_object;
	RpiFanServeAdaptor* m_adaptor;
	org::scotthamilton::RpiFanServe* m_proxy;
};
