#include "logger.hpp"

namespace logging
{

std::mutex Logger::logMutex;

Logger Log::debug("Debug", true);
Logger Log::trace("Trace", true);
Logger Log::info("Info", true);
Logger Log::warn("Warn", true);
Logger Log::error("Error", true);

} // namespace logging
