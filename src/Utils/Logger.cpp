#include "Logger.h"
#include <chrono>
#include <iomanip>
#include <sstream>

Logger& Logger::GetInstance() {
    static Logger instance;
    return instance;
}

void Logger::Initialize(const std::string& filename) {
    if (m_initialized) {
        return;
    }

    m_logFile.open(filename, std::ios::out | std::ios::app);
    if (m_logFile.is_open()) {
        m_initialized = true;
        LogInfo("Logger initialized successfully");
    } else {
        std::cerr << "Failed to initialize logger with file: " << filename << std::endl;
    }
}

void Logger::Log(LogLevel level, const std::string& message) {
    if (!m_initialized) {
        std::cerr << "Logger not initialized!" << std::endl;
        return;
    }

    std::string logEntry = "[" + GetTimestamp() + "] [" + GetLevelString(level) + "] " + message;

    // Write to file
    m_logFile << logEntry << std::endl;
    m_logFile.flush();

    // Also write to console for immediate feedback
    if (level == LogLevel::ERROR_ || level == LogLevel::FATAL) {
        std::cerr << logEntry << std::endl;
    } else {
        std::cout << logEntry << std::endl;
    }
}

void Logger::LogInfo(const std::string& message) {
    Log(LogLevel::INFO, message);
}

void Logger::LogWarning(const std::string& message) {
    Log(LogLevel::WARNING, message);
}

void Logger::LogError(const std::string& message) {
    Log(LogLevel::ERROR_, message);
}

void Logger::LogFatal(const std::string& message) {
    Log(LogLevel::FATAL, message);
}

void Logger::Shutdown() {
    if (m_initialized && m_logFile.is_open()) {
        LogInfo("Logger shutting down");
        m_logFile.close();
        m_initialized = false;
    }
}

Logger::~Logger() {
    Shutdown();
}

std::string Logger::GetLevelString(LogLevel level) {
    switch (level) {
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARN";
        case LogLevel::ERROR_: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

std::string Logger::GetTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::tm timeinfo;
    localtime_s(&timeinfo, &time_t);

    std::stringstream ss;
    ss << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();

    return ss.str();
}
