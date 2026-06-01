#include "Logger.hpp"

namespace cdm {

static LogCallback LOG_CALLBACK = nullptr;

void setLogCallback(LogCallback callback) {
    LOG_CALLBACK = std::move(callback);
}

void internalLog(LogLevel level, const std::string& message) {
    if (LOG_CALLBACK) {
        LOG_CALLBACK(level, message);
    }
}

}
