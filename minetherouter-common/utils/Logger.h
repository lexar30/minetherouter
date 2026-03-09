#pragma once

#include <iostream>
#include <string_view>
#include <mutex>

namespace mtr::utils 
{
	class Logger {
	public:
		enum class Level {
			UNDEFINED,
			DEBUG,
			INFO,
			WARNING,
			ERROR
		};

		Logger(const Logger&) = delete;
		Logger& operator=(const Logger&) = delete;

		static Logger& getInstance() {
			static Logger logger;
			return logger;
		}

		void log(Level level, std::string_view message) {
			std::lock_guard<std::mutex> lock(mtx);
			std::cout << "[" << levelToShortString(level) << "] " << message << std::endl;
		}

		static const char* levelToString(Level level) {
			switch (level) {
				case Level::UNDEFINED: return "UNDEFINED";
				case Level::DEBUG: return "DEBUG";
				case Level::INFO: return "INFO";
				case Level::WARNING: return "WARNING";
				case Level::ERROR: return "ERROR";
				default: return "UNKNOWN";
			}
		}

		static const char* levelToShortString(Level level) {
			switch (level) {
				case Level::UNDEFINED: return "U";
				case Level::DEBUG: return "D";
				case Level::INFO: return "I";
				case Level::WARNING: return "W";
				case Level::ERROR: return "E";
				default: return "?";
			}
		}

		static Level stringToLevel(std::string_view levelStr) {
			if (levelStr == "DEBUG" || levelStr == "D") return Level::DEBUG;
			if (levelStr == "INFO" || levelStr == "I") return Level::INFO;
			if (levelStr == "WARNING" || levelStr == "W") return Level::WARNING;
			if (levelStr == "ERROR" || levelStr == "E") return Level::ERROR;
			return Level::UNDEFINED;
		}

	private:
		Logger() = default;
		std::mutex mtx;
	};
}

#define MTR_LOGGER		 mtr::utils::Logger::getInstance()

#define LOG(level, msg)	 MTR_LOGGER.log(level, msg)

#define LOG_DEBUG(msg)   LOG(mtr::utils::Logger::Level::DEBUG, msg)
#define LOG_INFO(msg)    LOG(mtr::utils::Logger::Level::INFO, msg)
#define LOG_WARNING(msg) LOG(mtr::utils::Logger::Level::WARNING, msg)
#define LOG_ERROR(msg)   LOG(mtr::utils::Logger::Level::ERROR, msg)