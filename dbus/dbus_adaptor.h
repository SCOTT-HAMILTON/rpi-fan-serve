/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp -a dbus_adaptor /home/scott/GIT/rpi-fan-serve/dbus/org.scotthamilton.RpiFanServe.xml
 *
 * qdbusxml2cpp is Copyright (C) 2020 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#ifndef DBUS_ADAPTOR_H
#define DBUS_ADAPTOR_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * Adaptor class for interface org.scotthamilton.RpiFanServe
 */
class RpiFanServeAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.scotthamilton.RpiFanServe")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.scotthamilton.RpiFanServe\">\n"
"    <property access=\"readwrite\" type=\"x\" name=\"CacheLifeExpectancy\"/>\n"
"  </interface>\n"
        "")
public:
    RpiFanServeAdaptor(QObject *parent);
    virtual ~RpiFanServeAdaptor();

public: // PROPERTIES
    Q_PROPERTY(qlonglong CacheLifeExpectancy READ cacheLifeExpectancy WRITE setCacheLifeExpectancy)
    qlonglong cacheLifeExpectancy() const;
    void setCacheLifeExpectancy(qlonglong value);

public Q_SLOTS: // METHODS
Q_SIGNALS: // SIGNALS
};

#endif
