#include "TempsCtrl.h"
#include "SharedConfig.hpp"
#include <fstream>
#include <regex>
#include <filesystem>
#include <vector>
#include <future>

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
		"^(-?(?:[1-9][0-9]*)?[0-9]{4})-(1[0-2]|0[1-9])-(3[01]|0[1-9]|[12][0-9])T(2[0-3]|[01][0-9]):([0-5][0-9]):([0-5][0-9])(\\.[0-9]+)?(Z)?");
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
Json::Value linesChunk2TempsChunk(const std::vector<std::string>& linesChunk) {
	Json::Value chunk_temps(Json::arrayValue);
	for (const auto& line : linesChunk) {
		TempDataPoint data;
		if (extractMatches(line, data)) {
			chunk_temps.append(TempDataPoint2Json(data));
		}
	}
	return chunk_temps; 
}
Json::Value TempsCtrl::logFile2Json(const std::string& file) {
	std::ifstream ifs(file);
    if(!ifs) {
        throw std::runtime_error(file + ": " + std::strerror(errno));
	}
	ifs.seekg(0, std::ios::end);
	auto end = ifs.tellg();
	ifs.seekg(0, std::ios::beg);
    auto chunk_size = std::size_t(end - ifs.tellg())/Config::maxjobs;
    if (chunk_size == 0) {
		std::cerr << "[error] file " << file << " is empty\n";
        return jsonError("Log file is empty."); 
	} else {
		size_t currentChunkSize = 0;
		Json::Value temps(Json::arrayValue);
		std::vector<std::string> currentChunk;
		std::vector<std::future<Json::Value>> tasks;
		currentChunk.reserve(chunk_size/50);
		for(std::string line; getline(ifs, line );) {
			currentChunkSize += line.size();
			currentChunk.push_back(line);
			if (currentChunkSize >= chunk_size) {
				tasks.emplace_back(std::async(std::launch::async, [currentChunk]{
					return linesChunk2TempsChunk(currentChunk);
				}));
				currentChunkSize = 0;
				currentChunk.clear();
			}
		}
		if (currentChunk.size() > 0) {
			tasks.emplace_back(std::async(std::launch::async, [currentChunk]{
				return linesChunk2TempsChunk(currentChunk);
			}));
		}
		for (auto& task : tasks) {
			task.wait();
			auto chunk_temps = task.get();
			for (const auto& temp : chunk_temps) {
				temps.append(temp);
			}
		}
		return temps;
	}
}
