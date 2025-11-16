#pragma once
#include <string>
#include <fstream>
#include <iostream>

enum class LogLevel {
    INFO,
    WARNING,
    ERROR_,
    FATAL
};

class Logger {
public:
    static Logger& GetInstance();
    
    void Initialize(const std::string& filename = "game.log");
    void Log(LogLevel level, const std::string& message);
    void LogInfo(const std::string& message);
    void LogWarning(const std::string& message);
    void LogError(const std::string& message);
    void LogFatal(const std::string& message);
    void Shutdown();

private:
    Logger() = default;
    ~Logger();
    
    std::ofstream m_logFile;
    bool m_initialized = false;
    
    std::string GetLevelString(LogLevel level);
    std::string GetTimestamp();
};

// Convenience macros
#define LOG_INFO(msg) Logger::GetInstance().LogInfo(msg)
#define LOG_WARNING(msg) Logger::GetInstance().LogWarning(msg)
#define LOG_ERROR(msg) Logger::GetInstance().LogError(msg)
#define LOG_FATAL(msg) Logger::GetInstance().LogFatal(msg)