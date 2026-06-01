#pragma once

#include "Logger.hpp"

namespace cdm {

void internalLog(LogLevel level, const std::string& message);

}

#define CDM_DEBUG(...) cdm::internalLog(cdm::LogLevel::Debug, std::format(__VA_ARGS__))
#define CDM_INFO(...)  cdm::internalLog(cdm::LogLevel::Info,  std::format(__VA_ARGS__))
#define CDM_WARN(...)  cdm::internalLog(cdm::LogLevel::Warning, std::format(__VA_ARGS__))
#define CDM_ERR(...)   cdm::internalLog(cdm::LogLevel::Error, std::format(__VA_ARGS__))
