#pragma once

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <sys/file.h>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;
namespace SocketLockerConstants {
	constexpr const char LOCK_FILE[] =   "/var/lock/rpi-fan-serve.socket.lock";
	constexpr const char DBUS_LOCK_FILE[] =   "/var/lock/rpi-fan-serve-dbus.socket.lock";
	constexpr const char SOCKET_DIR[] = "/run/rpi-fan-serve";
	constexpr const char SOCKET_FILE[] = "rpi-fan-serve.sock";
	constexpr const int LOCK_FILE_MODE = 0644;

	static fs::path get_socket_file() noexcept {
		return fs::path(SOCKET_DIR) / fs::path(SOCKET_FILE);
	}
}

using namespace SocketLockerConstants;

template <const char* lockFile>
class SocketLocker {
public:
    SocketLocker() : m_lockFd(-1) {}

	// Checks the socket and tries to lock
	bool checkAndTryLock() noexcept {
		if (m_lockFd != -1) {
			std::cerr << "[error] lock file already successfully locked.\n";
			return true;
		} else {
			m_lockFd = try_lock(lockFile);
			if (m_lockFd == -1) {
				return false;
			}
			return check_clean_socket();
		}
	}
    ~SocketLocker() {
		if (m_lockFd != -1) {
			if (close(m_lockFd) == -1) {
				std::perror(
					(std::string(
						"[error] close(")+lockFile+") == -1").c_str());
			} else {
				std::cerr << "[debug] successfully closed lock "
						  << lockFile << '\n';
			}
		}
	}
private:
	int m_lockFd;
	// false on error, true on success
	bool try_delete (const fs::path& path) noexcept {
		std::error_code ec;
		if (fs::remove_all(path, ec) == static_cast<std::uintmax_t>(-1)) {
			std::cerr << "[error] fs::remove_all('" << path << "')"
					  << " failed with error code " << ec.value()
					  << " : " << ec.message() << '\n';
			return false;
		} else {
			return true;
		}
	}
	bool try_create_socket (const fs::path& socket) noexcept {
		int socket_fd = open(std::string(socket).c_str(),
				O_CREAT | O_RDWR,
				LOCK_FILE_MODE);
		if (socket_fd == -1) {
			std::perror(
				(std::string(
					"[error] open(")+std::string(socket)+") == -1")
				.c_str());
			return false;
		}
		if (close(socket_fd) == -1) {
			std::perror(
				(std::string(
					"[error] close(")+std::string(socket)+") == -1").c_str());
			return false;
		}
		return true;
	}
	bool is_good_chmod(const fs::perms p) noexcept {
		return
			((p & fs::perms::owner_read)   != fs::perms::none &&
			 (p & fs::perms::owner_write)  != fs::perms::none &&
			 (p & fs::perms::group_write)  == fs::perms::none &&
			 (p & fs::perms::others_write) == fs::perms::none);
	}
	std::string perms_to_string(const fs::perms p) noexcept {
		return std::string
				((p & fs::perms::owner_read) != fs::perms::none ? "r" : "-") +
				((p & fs::perms::owner_write) != fs::perms::none ? "w" : "-") +
				((p & fs::perms::owner_exec) != fs::perms::none ? "x" : "-") +
				((p & fs::perms::group_read) != fs::perms::none ? "r" : "-") +
				((p & fs::perms::group_write) != fs::perms::none ? "w" : "-") +
				((p & fs::perms::group_exec) != fs::perms::none ? "x" : "-") +
				((p & fs::perms::others_read) != fs::perms::none ? "r" : "-") +
				((p & fs::perms::others_write) != fs::perms::none ? "w" : "-") +
				((p & fs::perms::others_exec) != fs::perms::none ? "x" : "-");
	}

	/* Checks wether the lock exists, tries */
	/* to delete it if it's not valid (wrong chmod */
	/* or not a regular file) */
	void clean_lock (const fs::path& lock) noexcept {
		std::error_code ec;
		if (!fs::exists(lock, ec)) {
			if (ec.value() != 0) {
				std::cerr << "[error] fs::exists('" << lock << "')"
						  << " failed with error code " << ec.value()
						  << " : " << ec.message() << '\n';
			}
			return;
		} else {
			ec.clear();
			if (!fs::is_regular_file(lock, ec)) {
				if (ec.value() != 0) {
					std::cerr << "[error] fs::is_regular_file('" << lock << "')"
							  << " failed with error code " << ec.value()
							  << " : " << ec.message() << '\n';
				}
				try_delete(lock);
				return;
			} else {
				ec.clear();
				auto status = fs::status(lock, ec);
				if (ec.value() != 0) {
					std::cerr << "[error] fs::status('" << lock << "')"
							  << " failed with error code " << ec.value()
							  << " : " << ec.message() << '\n';
				}
				const auto perms = status.permissions();
				if (!is_good_chmod(perms)) {
					std::cerr << "[error] lock file "
							  << lock
							  << " has wrong permission "
							  << perms_to_string(perms)
							  << ", required to be only read-writeable by user"
							  << ", read-only to group and others "
							  << "(rw-r--r--) 644\n";
					try_delete(lock);
				}
			}
		}
	}

	// tries to lock the provided lock file,
	// returns -1 on failure, the lock file
	// descriptor otherwise
	int try_lock(const fs::path& lock) noexcept {
		clean_lock(lock);
		int lock_fd = open(std::string(lock).c_str(),
				O_CREAT | O_RDWR,
				LOCK_FILE_MODE);
		if (lock_fd == -1) {
			std::perror((std::string("[error] open(")+
					std::string(lock)+") == -1").c_str());
			return -1;
		}
		int rc = flock(lock_fd, LOCK_EX | LOCK_NB);
		if (rc == -1) {
			if (errno == EWOULDBLOCK) {
				std::cerr << "[fatal] only run one instance of rpi-fan-serve at a time.\n";
			} else {
				std::perror((std::string(
					"[error] locking socket lock with flock(")
						+ std::string(lock)
						+ ") == -1").c_str());
			}
			return -1;
		}
		return rc;
	}

	/* Checks wether the socke exists, tries */
	/* to replace it if it's not valid (wrong chmod */
	/* or not a regular file)
	 * returns false on error, true if socket is ready to use
	 * */
	bool check_clean_socket(const fs::path& socket) noexcept {
		std::error_code ec;
		if (!fs::exists(socket, ec)) {
			if (ec.value() != 0) {
				std::cerr << "[error] fs::exists('" << socket << "')"
						  << " failed with error code " << ec.value()
						  << " : " << ec.message() << '\n';
				return false;
			} else {
				return try_create_socket(socket);
			}
		} else {
			ec.clear();
			if (!fs::is_socket(socket, ec)) {
				if (ec.value() != 0) {
					std::cerr << "[error] fs::is_socket('" << socket << "')"
							  << " failed with error code " << ec.value()
							  << " : " << ec.message() << '\n';
				}
				std::cerr << "[debug] trying to delete " << socket
						  << " because it's not a socket.\n";
				try_delete(socket);
				return try_create_socket(socket);
			}
			ec.clear();
			auto status = fs::status(socket, ec);
			if (ec.value() != 0) {
				std::cerr << "[error] fs::status('" << socket << "')"
						  << " failed with error code " << ec.value()
						  << " : " << ec.message() << '\n';
			}
			const auto perms = status.permissions();
			if (!is_good_chmod(perms)) {
				std::cerr << "[error] socket file "
						  << socket
						  << " has wrong permission "
						  << perms_to_string(perms)
						  << ", required to be only read-writeable by user"
						  << ", read-only to group and others "
						  << "(rw-r--r--) 644\n";
				try_delete(socket);
				return try_create_socket(socket);
			}
			return true;
		}
	}
	// false on error, true on success
	bool try_create_directory (const fs::path& path) {
		std::error_code ec;
		if (!fs::create_directory(path, ec)) {
			if (ec.value() != 0) {
				std::cerr << "[error] fs::create_directory('" << path << "')"
						  << " failed with error code " << ec.value()
						  << " : " << ec.message() << '\n';
			}
			return false;
		} else {
			return true;
		}
	}
	// false on error, true on success
	bool check_clean_dir (const fs::path& path) noexcept {
		std::error_code ec;
		if (!fs::exists(path, ec)) {
			if (ec.value() != 0) {
				std::cerr << "[error] fs::exists('" << path << "')"
						  << " failed with error code " << ec.value()
						  << " : " << ec.message() << '\n';
			} else {
				return try_create_directory(path);
			}
		} else {
			ec.clear();
			if (!fs::is_directory(path, ec)) {
				if (ec.value() != 0) {
					std::cerr << "[error] fs::is_directory('" << path << "')"
							  << " failed with error code " << ec.value()
							  << " : " << ec.message() << '\n';
				}
				if (try_delete(path)) {
					return try_create_directory(path);
				} else {
					return false;
				}
			} else {
				return true;
			}
		}
		return true;
	}
	// Defaults to the SOCKET_FILE
	inline bool check_clean_socket() noexcept {
		auto socket_path = get_socket_file();
		if (!check_clean_dir(SOCKET_DIR)) {
			return false;
		} else {
			return check_clean_socket(socket_path);
		}
	}
};
