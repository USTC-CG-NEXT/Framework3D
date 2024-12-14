#pragma once

#include <chrono>
#include <functional>
#include <iostream>

#include "Logger/api.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
enum class Severity { None = 0, Debug, Info, Warning, Error, Fatal };

typedef std::function<void(Severity, char const*)> Callback;

namespace log {
LOGGER_API void SetMinSeverity(Severity severity);
LOGGER_API void SetCallback(Callback func);
LOGGER_API Callback GetCallback();
LOGGER_API void ResetCallback();

// Windows: enables or disables future log messages to be shown as
// MessageBox'es. This is the default mode. Linux: no effect, log messages are
// always printed to the console.
LOGGER_API void EnableOutputToMessageBox(bool enable);

// Windows: enables or disables future log messages to be printed to stdout or
// stderr, depending on severity. Linux: no effect, log messages are always
// printed to the console.
LOGGER_API void EnableOutputToConsole(bool enable);

// Windows: enables or disables future log messages to be printed using
// OutputDebugString. Linux: no effect, log messages are always printed to the
// console.
LOGGER_API void EnableOutputToDebug(bool enable);

// Windows: sets the caption to be used by the error message boxes.
// Linux: no effect.
LOGGER_API void SetErrorMessageCaption(const char* caption);

// Equivalent to the following sequence of calls:
// - EnableOutputToConsole(true);
// - EnableOutputToDebug(true);
// - EnableOutputToMessageBox(false);
LOGGER_API void ConsoleApplicationMode();

LOGGER_API void message(Severity severity, const char* fmt...);
LOGGER_API void debug(const char* fmt...);
LOGGER_API void info(const char* fmt...);
LOGGER_API void warning(const char* fmt...);
LOGGER_API void error(const char* fmt...);
LOGGER_API void fatal(const char* fmt...);

struct LOGGER_API ProfileScope {
    const char* name;
    ProfileScope(const char* name);
    ~ProfileScope();

   private:
    std::chrono::steady_clock::time_point begin_time;
};

LOGGER_API ProfileScope profile_scope(const char* fmt);

}  // namespace log

USTC_CG_NAMESPACE_CLOSE_SCOPE
