#pragma once

#include <drogon/HttpController.h>
#include <chrono>
#include <mutex>
#include "timercpp/timercpp.h"
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
	PermanentConfig::PermConfig m_config;
	std::mutex saveConfigMutex;
	std::mutex setCLEMutex;
	std::chrono::time_point<std::chrono::steady_clock> lastConfigSave;
	static constexpr int MAX_DELAY_BETWEEN_SAVES_MS = 5'000;
};
