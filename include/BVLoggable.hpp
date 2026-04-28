#pragma once
#include "spdlog/spdlog.h"
#include <memory>

class BVLoggable
{
private:
    std::shared_ptr<spdlog::logger> logger;

protected:
    template <typename... Args>
    void LogTrace(const char* fmt, Args&&... args) const
    {
        if (logger)
            SPDLOG_LOGGER_TRACE(logger, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void LogDebug(const char* fmt, Args&&... args) const
    {
        if (logger)
            SPDLOG_LOGGER_DEBUG(logger, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void LogInfo(const char* fmt, Args&&... args) const
    {
        if (logger)
            SPDLOG_LOGGER_INFO(logger, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void LogWarn(const char* fmt, Args&&... args) const
    {
        if (logger)
            SPDLOG_LOGGER_WARN(logger, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void LogError(const char* fmt, Args&&... args) const
    {
        if (logger)
            SPDLOG_LOGGER_ERROR(logger, fmt, std::forward<Args>(args)...);
    }

public:
    virtual ~BVLoggable() = default;

    void SetLogger(std::shared_ptr<spdlog::logger> _logger)
    {
        logger = std::move(_logger);
    }

    std::shared_ptr<spdlog::logger> GetLogger(void)
    {
        return this->logger;
    }

};