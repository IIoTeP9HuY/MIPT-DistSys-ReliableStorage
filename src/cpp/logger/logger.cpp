#include "logger.hpp"

namespace logging
{

std::mutex Logger::logMutex;

Logger Log::debug("Debug", false);
Logger Log::trace("Trace", false);
Logger Log::info("Info", true);
Logger Log::warn("Warn", true);
Logger Log::error("Error", true);

} // namespace logging
