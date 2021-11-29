#pragma once

#include <drogon/HttpController.h>
#include <chrono>
#include <mutex>
#include "timercpp/timercpp.h"
#include "zmq_dbus_server.hpp"
#include "configuration.hpp"

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
			std::cerr << "[log|ZMQ-DBUS-server] from TempsCtrl received: `"
					  << msg << "`\n";
			auto pr = parse_message(msg);
			try {
				auto cle = std::get<CacheLifeExpectancyMessage>(pr);
				m_parent.setCacheLifeExpectancy(cle.value);
			} catch (const std::bad_variant_access& ex) {}
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
	void saveCurrentConfig(bool check_delay = true);

private:
	static Json::Value logFile2Json(const std::string& file);
	static std::string offset2LogFilePath(size_t dayOffset);
	unsigned long getActiveLogFileCreationTimeMinutes() const;
	void setCacheTimer();
	void setSaveConfigTimer(int ms);
	DaysDataCache cache;
	std::mutex cacheMutex;
	Timer cacheTimer;
	Timer saveConfigTimer;
	unsigned long m_cacheLifeExpectancySeconds;
	CtrlDbusServer m_ctrlDbusServer;
	PermanentConfig::PermConfig m_config;
	std::mutex saveConfigMutex;
	std::mutex setCLEMutex;
	std::chrono::time_point<std::chrono::steady_clock> lastConfigSave;
	static constexpr int MAX_DELAY_BETWEEN_SAVES_MS = 5'000;
};
