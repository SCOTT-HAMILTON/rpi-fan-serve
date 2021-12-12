#pragma once

#include <QObject>

class RpiFanServeObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qlonglong CacheLifeExpectancy READ CacheLifeExpectancy WRITE setCacheLifeExpectancy NOTIFY CacheLifeExpectancyChanged)

public:
    explicit RpiFanServeObject(QObject *parent = nullptr);

    qlonglong CacheLifeExpectancy() const;
    void setCacheLifeExpectancy(qlonglong value);

signals:
    void CacheLifeExpectancyChanged();

private:
    qlonglong m_CacheLifeExpectancy;
};
