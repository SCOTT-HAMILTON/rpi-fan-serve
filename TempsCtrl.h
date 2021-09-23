#pragma once
#include <drogon/HttpController.h>
#include "timercpp/timercpp.h"

using namespace drogon;
using Days = std::chrono::duration<long, std::ratio<86400>>;

struct TempDataPoint {
	std::string date;
	int temp;
	int delta;
	int level;
};

struct DaysDataCache {
	unsigned long creationEpochHours;
	std::array<Json::Value, 6> data;
};

class TempsCtrl:public drogon::HttpController<TempsCtrl>
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

private:
	static Json::Value logFile2Json(const std::string& file);
	static std::string offset2LogFilePath(size_t dayOffset);
	DaysDataCache cache;
	std::mutex cacheMutex;
	Timer cacheTimer;
};
