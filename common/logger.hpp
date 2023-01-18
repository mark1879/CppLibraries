#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <iostream>

class TimeStamp {
public:
    TimeStamp() : micro_seconds_since_epoch_(0) 
    {}

    TimeStamp(int64_t micro_seconds_since_epoch)    
        : micro_seconds_since_epoch_(micro_seconds_since_epoch)
    {}

    static TimeStamp now()
    {
        return TimeStamp(time(nullptr));
    }

    std::string ToString() const
    {
         char buf[128] = {0};
        tm *tm_time = localtime(&micro_seconds_since_epoch_);
    
        snprintf(buf, 128, "%4d/%02d/%02d %02d:%02d:%02d", 
        tm_time->tm_year + 1900,
        tm_time->tm_mon + 1,
        tm_time->tm_mday,
        tm_time->tm_hour,
        tm_time->tm_min,
        tm_time->tm_sec);

        return buf;
    }

private:
    time_t micro_seconds_since_epoch_;
};

enum class LogLevel : std::uint8_t {
    kInfo,
    kWarning,
    kError,
    kFatal,
    kDebug
};

class Logger {
public:
    static Logger& Instance() {
        static Logger logger;
        return logger;
    }

    void set_log_level(LogLevel level) {
        log_level_ = level;
    }

    void Log(std::string msg) {
        switch (log_level_)
        {
        case LogLevel::kInfo:
            std::cout << "[INFO]";
            break;
        case LogLevel::kWarning:
            std::cout << "[WARNING]";
            break;
        case LogLevel::kError:
            std::cout << "[ERROR]";
            break;
        case LogLevel::kFatal:
            std::cout << "[FATAL]";
            break;
        case LogLevel::kDebug:
            std::cout << "[DEBUG]";
            break;
        default:
            break;
        }

        std::cout << TimeStamp::now().ToString() << " : " << msg << std::endl;
    }

private:
    LogLevel log_level_;
};

#define BUF_LEN 1024

#define LOG_INFO(log_msg_format, ...) \
    do { \
        Logger &logger = Logger::Instance(); \
        logger.set_log_level(LogLevel::kInfo); \
        char buf[BUF_LEN] = {0}; \
        snprintf(buf, BUF_LEN, log_msg_format, ##__VA_ARGS__); \
        logger.Log(buf); \
    } while(0) 

#define LOG_WARNING(log_msg_format, ...) \
    do { \
        Logger &logger = Logger::Instance(); \
        logger.set_log_level(LogLevel::kWarning); \
        char buf[BUF_LEN] = {0}; \
        snprintf(buf, BUF_LEN, log_msg_format, ##__VA_ARGS__); \
        logger.Log(buf); \
    } while(0) 

#define LOG_ERROR(log_msg_format, ...) \
    do \
    { \
        Logger &logger = Logger::Instance(); \
        logger.set_log_level(LogLevel::kError); \
        char buf[BUF_LEN] = {0}; \
        snprintf(buf, BUF_LEN, log_msg_format, ##__VA_ARGS__); \
        logger.Log(buf); \
    } while(0) 

#define LOG_FATAL(log_msg_format, ...) \
    do \
    { \
        Logger &logger = Logger::Instance(); \
        logger.set_log_level(LogLevel::kFatal); \
        char buf[BUF_LEN] = {0}; \
        snprintf(buf, BUF_LEN, log_msg_format, ##__VA_ARGS__); \
        logger.Log(buf); \
        exit(-1); \
    } while(0) 

#ifdef MUDEBUG
#define LOG_DEBUG(log_msg_format, ...) \
    do \
    { \
        Logger &logger = Logger::Instance(); \
        logger.set_log_level(LogLevel::kDebug); \
        char buf[BUF_LEN] = {0}; \
        snprintf(buf, BUF_LEN, log_msg_format, ##__VA_ARGS__); \
        logger.Log(buf); \
    } while(0) 
#else
    #define LOG_DEBUG(log_msg_format, ...)
#endif

#endif
