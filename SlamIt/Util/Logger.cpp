#include "Logger.hpp"

#include <chrono>
#include <format>
#include <fstream>
#include <vector>

Logger g_Logger;

namespace {
    constexpr const char* const levelStrings[] = {
        " DEBUG ",
        " INFO  ",
        "WARNING",
        " ERROR ",
        " FATAL "
    };
}

Logger::Logger() = default;

void Logger::SetFile(const std::string& fileName) {
    file = fileName;
}

void Logger::SetMinLevel(LogLevel level) {
    minLevel = level;
}

void Logger::Clear() const {
    std::ofstream logFile(file, std::ofstream::out | std::ofstream::trunc);
    logFile.close();
    if (logFile.fail())
        mError = true;
}

bool Logger::Error() {
    return mError;
}

void Logger::ClearError() {
    mError = false;
}

void Logger::write(LogLevel level, const std::string& txt) const {
#ifndef _DEBUG
    if (level < minLevel) return;
#endif
    std::ofstream logFile(file, std::ios_base::out | std::ios_base::app);

    const auto now = std::chrono::system_clock::now().time_since_epoch();
    logFile << std::format("[{:%H:%M:%S}] [{}] {}\n",
        std::chrono::duration_cast<std::chrono::milliseconds>(now),
        levelText(level),
        txt);

    logFile.close();
    if (logFile.fail())
        mError = true;
}

std::string Logger::levelText(LogLevel level) const {
    if (level > 4 || level < 0) {
        return "  UNK  ";
    }

    return levelStrings[level];
}
