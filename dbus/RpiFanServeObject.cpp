#include "RpiFanServeObject.h"
#include <QDebug>

RpiFanServeObject::RpiFanServeObject(QObject *parent)
    : QObject(parent)
{
}

qlonglong RpiFanServeObject::CacheLifeExpectancy() const {
	return m_CacheLifeExpectancy;
}

void RpiFanServeObject::setCacheLifeExpectancy(qlonglong value) {
	if (value < 60 || value > 100'000) {
		qDebug() << "[error] invalid cache life expectancy: " << value;
	} else {
		m_CacheLifeExpectancy = value;
		emit CacheLifeExpectancyChanged();
	}
}
