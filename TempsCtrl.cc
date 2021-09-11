#include "TempsCtrl.h"
#include "SharedConfig.hpp"
#include <fstream>
#include <regex>
#include <filesystem>

Json::Value jsonError(const std::string& errorMsg) {
	Json::Value value;
	value["type"] = "Error";
	value["message"] = errorMsg;
	return value;
}
HttpResponsePtr jsonErrorResponse(const std::string& errorMsg) {
	return HttpResponse::newHttpJsonResponse(
		jsonError(errorMsg)
	);
}

void TempsCtrl::handleTempsRequest(
		const HttpRequestPtr &req,
		std::function<void (const HttpResponsePtr &)> &&callback,
		const std::string& dayOffset) const
{
	size_t intDayOffset = 0;
	try {
		// Just to check if it's a valid unsigned integer.
		intDayOffset = std::stoul(dayOffset, nullptr, 10);
	} catch (std::invalid_argument&) {
		callback(jsonErrorResponse("Invalid day offset `"+dayOffset+"`."));
		return;
	} catch (std::out_of_range&) {
		callback(jsonErrorResponse("Out of range day offset `"+dayOffset+"`."));
		return;
	}
	std::string logFilePath(Config::logFilePath);
	if (intDayOffset > 0) {
		logFilePath += "."+dayOffset;
	}
	if (!std::filesystem::exists(logFilePath)) {
		callback(jsonErrorResponse("Log file "+dayOffset+" doesn't exist"));
		return;
	} else {
		auto temps = logFile2Json(logFilePath);
		std::cerr << "[log] requested day offset: " << dayOffset << '\n';
		auto resp = HttpResponse::newHttpJsonResponse(temps);
		callback(resp);
	}
}

inline std::string getFirstMatch(const std::string& line, const std::regex& regex) {
	std::sregex_iterator it(line.begin(), line.end(), regex);
	std::sregex_iterator end;
	if (it != end) {
		return std::smatch(*it).str();
	} else {
		return "";
	}
}
std::ostream& operator<<(std::ostream &out, const TempDataPoint &data)
{
    out << "TempDataPoint(date=" << data.date <<
	       ",temp=" << data.temp <<
	       ",delta=" << data.delta <<
	       ",level=" << data.level << ")";
    return out;
}
inline Json::Value TempDataPoint2Json(const TempDataPoint& data) {
	Json::Value value;
	value["date"] = data.date;
	value["temp"] = data.temp;
	value["delta"] = data.delta;
	value["level"] = data.level;
	return value;
}
inline bool extractMatches(const std::string& line, TempDataPoint& result) {
	const std::regex dateRegex(
		"^([0-9]{2}/){2}[0-9]{2} [0-9]{2}:[0-9]{2}");
	const std::regex tempRegex("with [0-9]*°C");
	const std::regex deltaRegex("Δ[0-9]*°C");
	const std::regex levelRegex("level is [0-9]*");
	auto date = getFirstMatch(line, dateRegex);
	if (date == "") {
		return false;
	} else {
		result.date = date;
		{
			auto temp = getFirstMatch(line, tempRegex);
			temp = temp.substr(5, temp.size()-8);
			try {
				auto tempInt = std::stoi(temp, nullptr, 10);
				result.temp = tempInt;
			} catch (std::invalid_argument&) {
				std::cerr << "[error] bad line, invalid argument temp: `" << temp
					<< "` `"<< line << "`\n";
				return false;
			} catch (std::out_of_range&) {
				std::cerr << "[error] bad line, out of range temp: `" << temp
					<< "` `"<< line << "`\n";
				return false;
			}
		}
		{
			std::string delta = getFirstMatch(line, deltaRegex);
			delta = delta.substr(2, delta.size()-5);
			try {
				auto deltaInt = std::stoi(delta.c_str(), nullptr, 10);
				result.delta = deltaInt;
			} catch (std::invalid_argument&) {
				std::cerr << "[error] bad line, invalid argument delta: `" << delta
					<< "` `"<< line << "`\n";
				return false;
			} catch (std::out_of_range&) {
				std::cerr << "[error] bad line, out of range delta: `" << delta
					<< "` `"<< line << "`\n";
				return false;
			}
		}
		{
			auto level = getFirstMatch(line, levelRegex);
			level = level.substr(9, level.size()-12);
			try {
				auto levelInt = std::stoi(level, nullptr, 10);
				result.level = levelInt;
			} catch (std::invalid_argument&) {
				std::cerr << "[error] bad line, invalid argument level: `" << level
					<< "` `"<< line << "`\n";
				return false;
			} catch (std::out_of_range&) {
				std::cerr << "[error] bad line, out of range level: `" << level
					<< "` `"<< line << "`\n";
				return false;
			}
		}
		return true;
	}
}

Json::Value TempsCtrl::logFile2Json(const std::string& file) {
	std::ifstream ifs(file);
    if(!ifs) {
        throw std::runtime_error(file + ": " + std::strerror(errno));
	}
	ifs.seekg(0, std::ios::end);
	auto end = ifs.tellg();
	ifs.seekg(0, std::ios::beg);
    auto size = std::size_t(end - ifs.tellg());
	Json::Value temps(Json::arrayValue);
    if (size == 0) {
		std::cerr << "[error] file " << file << " is empty\n";
        return jsonError("Log file is empty."); 
	} else {
		for(std::string line; getline(ifs, line );) {
			TempDataPoint data;
			if (extractMatches(line, data)) {
				/* std::cerr << "line: " << data << '\n'; */
				temps.append(TempDataPoint2Json(data));
			}
		}
		return temps;
	}
}
