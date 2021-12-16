#pragma once

#include <iostream>
#include <filesystem>
#include <fstream>
#include <libconfig.h++>

using namespace std;
using namespace libconfig;

namespace fs = std::filesystem;

namespace PermanentConfig {
	struct PermConfig {
		int cache_life_expectancy;
		friend std::ostream& operator<<(std::ostream& os, const PermConfig& c);
	};
	constexpr const char* CONFIG_FILE  = "config.cfg";
	constexpr const PermConfig DefaultConfig = {
		// cache_life_expectancy in seconds
		1'000
	};
	constexpr const char* CacheLifeExpectancyConfigKey = "cacheLifeExpectancySeconds";
	
	std::ostream& operator<<(std::ostream& os, const PermConfig& c) {
		os << "PermConfig(cache_life_expectancy=" << c.cache_life_expectancy << ")";
		return os;
	}
	enum CheckReturn {
		FS_ERROR,
		DONT_EXIST,
		WRONG_FHS,
		SUCCESS
	};
	enum LoadConfigReturn {
		NO_CONFIG,
		ERROR,
		INVALID_CONFIG,
		LOAD_SUCCESS
	};
	const std::string configFilePath() noexcept {
		return "/srv/rpi-fan-serve/" + std::string(CONFIG_FILE);
	};
	CheckReturn check_dir(const fs::path& p) noexcept {
		std::error_code ec;
		if (!fs::exists(p, ec)) {
			if (ec.value() != 0) {
				std::cerr << "[error] fs::exists('" << p << "')"
						  << " failed with error code " << ec.value()
						  << " : " << ec.message() << '\n';
				return FS_ERROR;
			} else {
				return DONT_EXIST;
			}
		}
		ec.clear();
		if (!fs::is_directory(p, ec)) {
			if (ec.value() != 0) {
				std::cerr << "[error] fs::is_directory('" << p << "')"
						  << " failed with error code " << ec.value()
						  << " : " << ec.message() << '\n';
				return FS_ERROR;
			} else {
				return DONT_EXIST;
			}
		} else {
			return SUCCESS;
		}
	}
	CheckReturn check_config_file_system() noexcept {
		if (auto cr = check_dir("/srv"); cr != SUCCESS) {
			if (cr == DONT_EXIST) {
				std::cerr << "[error] /srv isn't a directory,"
						  << " rpi-fan-serve needs a valid FHS to run."
						  << " It won't be able to preserve configuration"
						  << " on next launch.\n";
				return WRONG_FHS;
			} else {
				return FS_ERROR;
			}
		}
		if (auto cr = check_dir("/srv/rpi-fan-serve"); cr != SUCCESS) {
			if (cr == DONT_EXIST) {
				std::error_code ec;
				if (!fs::create_directory("/srv/rpi-fan-serve", ec)) {
					std::cerr << "[error]"
							  << " fs::create_directory(/srv/rpi-fan-serve"
							  << " failed";
					if (ec.value() != 0) {
						std::cerr << " with error code " << ec.value()
								  << ": " << ec.message() << '.';
					} else {
						std::cerr << ".";
					}
					std::cerr << " rpi-fan-serve won't be able to preserve"
							  << " configuration on next launch.\n";
					return FS_ERROR;
				} else {
					std::cerr << "[log] created config directory /srv/rpi-fan-serve.\n";
				}
			} else {
				return FS_ERROR;
			}
		}
		auto config_file_path = configFilePath();
		std::error_code ec;
		if (!fs::exists(config_file_path, ec)) {
			if (ec.value() != 0) {
				std::cerr << "[error] fs::exists('" << config_file_path << "')"
						  << " failed with error code " << ec.value()
						  << " : " << ec.message() << '\n';
				return FS_ERROR;
			} else {
				return DONT_EXIST;
			}
		}
		ec.clear();
		if (!fs::is_regular_file(config_file_path, ec)) {
			if (ec.value() != 0) {
				std::cerr << "[error] fs::is_regular_file"
					<< " failed with error code " << ec.value()
					<< ": " << ec.message() << '\n';
				return FS_ERROR;
			} else {
				std::cerr << "not regular file because don't exist\n";
				return DONT_EXIST;
			}
		}
		return SUCCESS;
	}
	// true on success, false on failure
	bool parse_config_file(
			PermConfig& config,
			std::string configFile) noexcept {
		Config cfg;
		try {
			cfg.readFile(configFile);
		} catch (const FileIOException &fioex) {
			std::cerr << "[error] failed to read config file.\n";
			return false;
		} catch (const ParseException &pex) {
			std::cerr << "[error] parsing failed at " << pex.getFile()
				<< ":" << pex.getLine()
				<< " - " << pex.getError() << ".\n";
			return false;
		}
		int cacheLifeExpectancy;
		if (!cfg.lookupValue(CacheLifeExpectancyConfigKey,
					cacheLifeExpectancy)) {
			std::cerr << "[error] invalid config file,"
				<< " no cacheLifeExpectancy.\n";
			return false;
		} else {
			config.cache_life_expectancy = cacheLifeExpectancy;
			return true;
		}
	}
	LoadConfigReturn load_config(PermConfig& config) noexcept {
		if (auto crv = check_config_file_system(); crv != SUCCESS) {
			if (crv == DONT_EXIST) {
				std::cerr << "[log] no config file at "
						  << configFilePath() << ".\n";
				return NO_CONFIG;
			} else {
				return ERROR;
			}
		}
		std::string config_file_path = configFilePath();
		if (!parse_config_file(config, config_file_path)) {
			return INVALID_CONFIG;
		} else {
			return LOAD_SUCCESS;
		}
	}
	bool save_config(
				const PermConfig& config,
				std::string configFile) noexcept {
		std::error_code ec;
		if (!fs::exists(configFile, ec)) {
			if (ec.value() != 0) {
				std::cerr << "[error] fs::exists('" << configFile << "')"
						  << " failed with error code " << ec.value()
						  << " : " << ec.message() << '\n';
				return false;
			} else {
				std::ofstream file(configFile, std::ios::trunc);
				if (!file.is_open()) {
					std::cerr << "[error] failed to create config file at "
							  << configFile << ".\n";
					return false;
				}
			}
		}
		Config cfg;
		cfg.setOptions(Config::OptionFsync);
		try {
			cfg.readFile(configFile);
		} catch (const FileIOException &fioex) {
			std::cerr << "[error] failed to read config file.\n";
			return false;
		} catch (const ParseException &pex) {
			std::cerr << "[error] parsing failed at " << pex.getFile()
				<< ":" << pex.getLine()
				<< " - " << pex.getError() << ".\n";
			return false;
		}
		Setting &root = cfg.getRoot();
		try {
			root.lookup(CacheLifeExpectancyConfigKey)
					= config.cache_life_expectancy;
		} catch(SettingNotFoundException& e) {
			try {
				root.add(CacheLifeExpectancyConfigKey, Setting::TypeInt)
					= config.cache_life_expectancy;
			} catch (SettingNameException& e) {
				std::cerr << "[error] SettingNameException,"
						  << " failed to save config " << config
						  << " to file " << configFile << ".\n";
			}
		}
		try {
			cfg.writeFile(configFile);
			std::cerr << "[log] config written to " << configFile << ".\n";
			return true;
		} catch(const FileIOException &fioex) {
			std::cerr << "[error] failed to write config file.\n";
			return false;
		}
	}
}
