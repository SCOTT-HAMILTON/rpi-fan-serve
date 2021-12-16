#pragma once

#include <QThread>
#include <QDebug>
#include <atomic>
#include <string>

class ZmqPairQThread : public QThread
{
    Q_OBJECT

public:
    ZmqPairQThread(std::string_view TAG, QObject *parent = nullptr) :
		QThread(parent), m_running(true), mTAG(TAG) {}
    ~ZmqPairQThread() {
		m_running.store(false);
		wait();
	}
	void log(std::string_view msg) const noexcept {
		qDebug() << "[log|" << mTAG.c_str() << "] "
				 << std::string(msg.data(), msg.size()).c_str()
				 << '\n';
	}

protected:
    virtual void run() = 0;
	std::atomic_bool m_running;

private:
	const std::string mTAG;
};
