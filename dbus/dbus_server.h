#pragma once

#include <QObject>
#include "RpiFanServeObject.h"
#include "dbus_proxy.h"
#include "dbus_adaptor.h"

class DbusServer: public QObject
{
    Q_OBJECT
public:
    DbusServer(QObject *parent = nullptr);
    ~DbusServer();

public slots:
	void onCacheLifeExpectancyChanged();	

private:
	RpiFanServeObject* m_object;
	RpiFanServeAdaptor* m_adaptor;
	org::scotthamilton::RpiFanServe* m_proxy;
};
