#include <QCoreApplication>
#include <QDebug>

#include "dbus_server.h"

DbusServer::DbusServer(QObject *parent)
    : QObject(parent), m_ctrlDbusClient(*this)
{
    auto connection = QDBusConnection::systemBus();
	if (!connection.isConnected()){
		qDebug() << "[fatal] can't connect to D-Bus system bus";
		qDebug() << connection.lastError().message();
		QCoreApplication::quit();
		return;
	}
	m_object = new RpiFanServeObject(this);
	m_adaptor = new RpiFanServeAdaptor(m_object);
	connection.registerObject("/org/scotthamilton/rpifanserver", m_object);
    connection.registerService("org.scotthamilton.RpiFanServe");

    m_proxy = new org::scotthamilton::RpiFanServe(
			"org.scotthamilton.RpiFanServe",
			"/org/scotthamilton/rpifanserver",
			QDBusConnection::sessionBus(), this);
	connect(m_object, &RpiFanServeObject::CacheLifeExpectancyChanged,
			this, &DbusServer::onCacheLifeExpectancyChanged);
	qDebug() << connection.lastError().message();
	m_ctrlDbusClient.start();
}

DbusServer::~DbusServer()
{}

void DbusServer::onCacheLifeExpectancyChanged() {
	auto newValue = m_adaptor->cacheLifeExpectancy();
	qDebug() << "[log] new CacheLifeExpectancyChanged -> "
			 << newValue;
	m_ctrlDbusClient.queue_send(
		ZmqConstants::NEW_CACHE_LIFE_EXPECTANCY_KEY
			+ std::string(":")
			+ std::to_string(newValue)
	);
}
