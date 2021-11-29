#include "TempsCtrl.h"
#include "SharedConfig.hpp"
#include "configuration.hpp"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <regex>
#include <filesystem>
#include <vector>
#include <future>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <algorithm>

TempsCtrl::TempsCtrl() :
	drogon::HttpController<TempsCtrl>(),
	cacheTimer(),
	saveConfigTimer(),
	/* m_cacheLifeExpectancySeconds(2*3600) */
	m_cacheLifeExpectancySeconds(
				PermanentConfig::DefaultConfig.cache_life_expectancy),
	m_ctrlDbusServer(*this),
	lastConfigSave(std::chrono::steady_clock::now())
{
	{
		using namespace PermanentConfig;
		if (auto lrv = load_config(m_config); lrv != LOAD_SUCCESS) {
			m_config = DefaultConfig;
			if (lrv == NO_CONFIG) {
				std::cerr << "[log] config file doesn't exist,"
						  << " creating default config...\n";
				std::lock_guard<std::mutex> guard(saveConfigMutex);
				save_config(m_config, PermanentConfig::configFilePath());
			} else {
				std::cerr << "[error] couldn't load config...\n";
			}
		} else {
			std::cerr << "[log] config loaded successfully : "
					  << m_config << '\n';
			m_cacheLifeExpectancySeconds = m_config.cache_life_expectancy;
		}
	}
	cache.creationEpochMinutes = 0;
	updateCache();
	setCacheTimer();
	m_ctrlDbusServer.start();
}

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
	auto logFilePath = offset2LogFilePath(intDayOffset);
	if (logFilePath != "") {
		auto temps = logFile2Json(logFilePath);
		std::cerr << "[log] requested day offset: " << dayOffset << '\n';
		auto resp = HttpResponse::newHttpJsonResponse(temps);
		callback(resp);
	} else {
		callback(jsonErrorResponse("Log file "+dayOffset+" doesn't exist"));
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
    auto chunk_size = std::size_t(end - ifs.tellg())/SharedConfig::maxjobs;
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

std::string TempsCtrl::offset2LogFilePath(size_t dayOffset) {
	std::string logFilePath(SharedConfig::logFilePath);
	if (dayOffset > 0) {
		logFilePath += "."+std::to_string(dayOffset);
	}
	if (std::filesystem::exists(logFilePath)) {
		return std::filesystem::absolute(
			std::filesystem::path(logFilePath)
		);
	} else {
		return "";
	}
}

unsigned long TempsCtrl::getActiveLogFileCreationTimeMinutes() const {
	auto activeLogFile = offset2LogFilePath(0);
	struct statx buf;
	statx(
		AT_FDCWD,
		activeLogFile.c_str(),
		AT_STATX_SYNC_AS_STAT|AT_SYMLINK_NOFOLLOW,
		STATX_ALL, &buf);
	auto seconds = std::chrono::seconds(buf.stx_btime.tv_sec);
	std::chrono::time_point<std::chrono::system_clock> t(seconds);
	return std::chrono::duration_cast<std::chrono::minutes>(
		t.time_since_epoch()
	).count();
}

void TempsCtrl::setCacheTimer() {
	unsigned long cacheTimerInterval =
			std::max(static_cast<unsigned long>(
						m_cacheLifeExpectancySeconds*0.005f*1'000.0f),
					10'000UL);
	std::cerr << "[debug] cacheTimerIntervalMs=" << cacheTimerInterval << '\n';
	cacheTimer.setInterval([&]() {
		if (isCacheNeedingUpdate()) {
			std::cerr << "[log] timer triggered cache update.\n";
			updateCache();
		} else {
			std::cerr << "[log] timer update disabled, cache is still valid.\n";
		}
	}, cacheTimerInterval);
}

void TempsCtrl::setSaveConfigTimer(int ms) {
	auto timeout = std::max(ms, 1'000);
	saveConfigTimer.setTimeout([&]() {
		saveCurrentConfig(false);
	}, timeout);
}

void TempsCtrl::updateCache() {
    std::lock_guard<std::mutex> guard(cacheMutex);
	std::cerr << "[log] cache is expired, updating...\n";
	for (auto offset = 0; offset < 7; ++offset) {
		auto logFile = offset2LogFilePath(offset);
		if (logFile != "") {
			auto temps = logFile2Json(logFile);
			cache.data[offset] = temps;
		} else {
			cache.data[offset] = Json::Value(Json::arrayValue);
		}
	}
	std::cerr << "[log] cache updated.\n";
	auto currentEpoch = std::chrono::duration_cast<std::chrono::minutes>(
			std::chrono::system_clock::now().time_since_epoch()).count();
	cache.creationEpochMinutes = currentEpoch;
}

void TempsCtrl::handleAllTempsRequest(
		const HttpRequestPtr &req,
		std::function<void (const HttpResponsePtr &)> &&callback)
{
	Json::Value alltemps(Json::arrayValue);
	if (isCacheExpired()) {
		std::cerr << "[log] request triggered cache update.\n";
		updateCache();
	} else {
		std::cerr << "[log] cache is up to date\n";
	}
	std::lock_guard<std::mutex> guard(cacheMutex);
	for (const auto& temps : cache.data) {
		alltemps.append(temps);
	}
	std::cerr << "[log] sending answer...\n";
	callback(HttpResponse::newHttpJsonResponse(alltemps));
}

std::string formatEpochMinutes(unsigned long minutes) {
    const auto minutes_duration = std::chrono::minutes(minutes);
	std::chrono::time_point<std::chrono::system_clock> t(minutes_duration);
    const std::time_t t_c = std::chrono::system_clock::to_time_t(t);
	std::stringstream s;
    s.imbue(std::locale(""));
    s << std::put_time(std::localtime(&t_c), "%F %T");
	return s.str();
}

bool TempsCtrl::isCacheExpired() const
{
	auto currentEpoch = std::chrono::duration_cast<std::chrono::minutes>(
			std::chrono::system_clock::now().time_since_epoch()).count();
	auto activeLogFileCreationTime = getActiveLogFileCreationTimeMinutes();
	if (cache.creationEpochMinutes < activeLogFileCreationTime) {
		std::cerr << "[debug] cache expired, log files have been rotated at "
			<< formatEpochMinutes(activeLogFileCreationTime)
			<< " and cache last update was at "
			<< formatEpochMinutes(cache.creationEpochMinutes)
			<< ".\n";
		return true;
	} else {
		if (currentEpoch >= cache.creationEpochMinutes+m_cacheLifeExpectancySeconds/60) {
			std::cerr << "[debug] ["
				<< formatEpochMinutes(currentEpoch)
				<< "] cache is expired, its last update was at "
				<< formatEpochMinutes(cache.creationEpochMinutes)
				<< ".\n";
			return true;
		} else {
			std::cerr << "[debug] ["
				<< formatEpochMinutes(currentEpoch)
				<< "] cache is still valid, its last update was at "
				<< formatEpochMinutes(cache.creationEpochMinutes)
				<< ".\n";
			return false;
		}
	}
}
bool TempsCtrl::isCacheNeedingUpdate() const
{
	auto currentEpoch = std::chrono::duration_cast<std::chrono::minutes>(
			std::chrono::system_clock::now().time_since_epoch()).count();
	auto activeLogFileCreationTime = getActiveLogFileCreationTimeMinutes();
	if (cache.creationEpochMinutes < activeLogFileCreationTime) {
		std::cerr << "[debug] cache needs update, log files have been rotated at "
			<< formatEpochMinutes(activeLogFileCreationTime)
			<< " and cache last update was at "
			<< formatEpochMinutes(cache.creationEpochMinutes)
			<< ".\n";
		return true;
	} else {
		unsigned long cache_update_time_minutes = (m_cacheLifeExpectancySeconds/60L)*0.95;
		std::cerr << "[debug] cache_update_time_minutes = " << cache_update_time_minutes << '\n';
		if (currentEpoch >= cache.creationEpochMinutes+cache_update_time_minutes) {
			std::cerr << "[debug] ["
				<< formatEpochMinutes(currentEpoch)
				<< "] cache needs update, its last update was at "
				<< formatEpochMinutes(cache.creationEpochMinutes)
				<< ".\n";
			return true;
		} else {
			std::cerr << "[debug] ["
				<< formatEpochMinutes(currentEpoch)
				<< "] cache doesn't need any update, its last update was at "
				<< formatEpochMinutes(cache.creationEpochMinutes)
				<< ".\n";
			return false;
		}
	}
}

void TempsCtrl::setCacheLifeExpectancy(int seconds) {
	std::lock_guard<std::mutex> guard(setCLEMutex);
	std::cerr << "[log] changed cache life expectancy "
			  << m_cacheLifeExpectancySeconds << " -> "
			  << seconds << '\n';
	m_cacheLifeExpectancySeconds = seconds;
	m_config.cache_life_expectancy = m_cacheLifeExpectancySeconds;
	saveCurrentConfig();
	setCacheTimer();
}

void TempsCtrl::saveCurrentConfig(bool check_delay) {
	auto now = std::chrono::steady_clock::now();
	auto durationMs =
		std::chrono::duration_cast<std::chrono::milliseconds>
			(now - lastConfigSave).count();
	if (check_delay) {
		std::lock_guard<std::mutex> guard(saveConfigMutex);
		if (durationMs < MAX_DELAY_BETWEEN_SAVES_MS) {
			setSaveConfigTimer(MAX_DELAY_BETWEEN_SAVES_MS - durationMs);
		} else {
			lastConfigSave = now;
			save_config(m_config, PermanentConfig::configFilePath());
		}
	} else {
		lastConfigSave = now;
		save_config(m_config, PermanentConfig::configFilePath());
	}
}
