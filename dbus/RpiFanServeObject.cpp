#include "RpiFanServeObject.h"

RpiFanServeObject::RpiFanServeObject(QObject *parent)
    : QObject(parent)
{
}

qlonglong RpiFanServeObject::CacheLifeExpectancy() const {
	return m_CacheLifeExpectancy;
}

void RpiFanServeObject::setCacheLifeExpectancy(qlonglong value) {
	m_CacheLifeExpectancy = value;
	emit CacheLifeExpectancyChanged();
}
