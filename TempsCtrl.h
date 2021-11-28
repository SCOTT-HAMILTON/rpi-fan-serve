#pragma once
#include <drogon/HttpController.h>
#include "timercpp/timercpp.h"
#include "zmq_dbus_server.hpp"

using namespace drogon;
using Days = std::chrono::duration<long, std::ratio<86400>>;

struct TempDataPoint {
	std::string date;
	int temp;
	int delta;
	int level;
};

struct DaysDataCache {
	unsigned long creationEpochMinutes;
	std::array<Json::Value, 7> data;
};

class TempsCtrl: public drogon::HttpController<TempsCtrl>
{
	class CtrlDbusServer: public ZmqDbusServer {
	public:
		CtrlDbusServer(TempsCtrl& parent) : m_parent(parent) {}
	protected:
		void receiveCallback(const std::string& msg) override {
		}
		void trySendCallback(zmq::socket_t& socket) override {
			/* std::cerr << "[debug] sending nothing but can access " << m_parent.m_cacheLifeExpectancySeconds << "\n"; */
		}
		TempsCtrl& m_parent;
	};

public:
	TempsCtrl();
	METHOD_LIST_BEGIN
		ADD_METHOD_TO(TempsCtrl::handleTempsRequest,"/temps?dayOffset={1}",Get);
		ADD_METHOD_TO(TempsCtrl::handleAllTempsRequest,"/all_temps",Get);
	METHOD_LIST_END
	void handleTempsRequest(const HttpRequestPtr &req,
			std::function<void (const HttpResponsePtr &)> &&callback, const std::string& dayOffset) const;
	void updateCache();
	void handleAllTempsRequest(const HttpRequestPtr &req,
			std::function<void (const HttpResponsePtr &)> &&callback);
	bool isCacheExpired() const;
	bool isCacheNeedingUpdate() const;
	void setCacheLifeExpectancy(int seconds);

private:
	static Json::Value logFile2Json(const std::string& file);
	static std::string offset2LogFilePath(size_t dayOffset);
	unsigned long getActiveLogFileCreationTimeMinutes() const;
	void setCacheTimer();
	DaysDataCache cache;
	std::mutex cacheMutex;
	Timer cacheTimer;
	unsigned long m_cacheLifeExpectancySeconds;
	CtrlDbusServer m_ctrlDbusServer;
};
