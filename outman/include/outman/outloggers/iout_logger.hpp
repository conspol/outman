#pragma once

#include <string>

class IOutLogger {
public:
    virtual ~IOutLogger() = default;
    virtual void Log(const std::string& message, const void* sender) = 0;
};
