#pragma once
#include <drogon/HttpController.h>

using namespace drogon;

struct TempDataPoint {
	std::string date;
	int temp;
	int delta;
	int level;
};

class TempsCtrl:public drogon::HttpController<TempsCtrl>
{
public:
	METHOD_LIST_BEGIN
		ADD_METHOD_TO(TempsCtrl::handleTempsRequest,"/temps?dayOffset={1}",Get);
		ADD_METHOD_TO(TempsCtrl::handleAllTempsRequest,"/all_temps",Get);
	METHOD_LIST_END
	void handleTempsRequest(const HttpRequestPtr &req,
			std::function<void (const HttpResponsePtr &)> &&callback, const std::string& dayOffset) const;
	void handleAllTempsRequest(const HttpRequestPtr &req,
			std::function<void (const HttpResponsePtr &)> &&callback) const;

private:
	static Json::Value logFile2Json(const std::string& file);
	static std::string offset2LogFilePath(size_t dayOffset);
};
