#pragma once

#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

namespace PermanentConfig {
	constexpr const char* CONFIG_FILE  = "config.json";
	struct PermConfig {
		int cache_life_expectancy;
		friend std::ostream& operator<<(std::ostream& os, const PermConfig& c);
	};
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
		std::error_code ec;
		if (!fs::is_regular_file(
					"/srv/rpi-fan-serve/" + std::string(CONFIG_FILE)), ec) {
			if (ec.value() != 0) {
				std::cerr << "[error] fs::is_regular_file"
						  << " failed with error code " << ec.value()
						  << ": " << ec.message() << '\n';
				return FS_ERROR;
			} else {
				return DONT_EXIST;
			}
		}
		return SUCCESS;
	}
	// true on success, false on failure
	bool load_config(PermConfig& config) noexcept {
		if (auto crv = check_config_file_system(); crv != SUCCESS) {
			if (crv == DONT_EXIST) {
				std::cerr << "[log] creating config file: " << CONFIG_FILE << ".\n";
			} else {
				return false;
			}
		}
		return true;
	}
}
