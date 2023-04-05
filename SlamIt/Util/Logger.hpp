#pragma once
#include <format>
#include <string>

#define LOG(level, fmt, ...) \
    g_Logger.Write(level, fmt, __VA_ARGS__)

enum LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
};

class Logger {

public:
    Logger();
    void SetFile(const std::string& fileName);
    void SetMinLevel(LogLevel level);
    void Clear() const;
    bool Error();
    void ClearError();

    template <typename... Args>
    void Write(LogLevel level, std::string_view fmt, Args&&... args) const {
        try {
            write(level, std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...)));
        }
        catch (const std::exception& ex) {
            write(ERROR, std::format("Failed to format: [{}], {}", fmt, ex.what()));
        }
    }

private:
    std::string levelText(LogLevel level) const;
    void write(LogLevel, const std::string& txt) const;

    mutable bool mError = false;
    std::string file = "";
    LogLevel minLevel = INFO;
};

extern Logger g_Logger;
