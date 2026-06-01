#pragma once

#include <functional>
#include <string>

namespace cdm {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

using LogCallback = std::function<void(LogLevel level, const std::string& message)>;

void setLogCallback(LogCallback callback);

}
