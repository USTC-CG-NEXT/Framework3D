#pragma once

#include <functional>
#include <iostream>

#include "Logger/api.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
enum class Severity { None = 0, Debug, Info, Warning, Error, Fatal };

inline void logging(
    const std::string& log_content,
    Severity level = Severity::Warning)
{
    switch (level) {
        case Severity::Info:
#ifndef NDEBUG
            std::cout << "[[USTC_CG Info]]: " << log_content << std::endl;
#endif
            break;

        case Severity::Warning:
            std::cout << "\x1B[33m[[USTC_CG Warning]]: " << log_content
                      << "\x1B[0m" << std::endl;
            break;
        case Severity::Error:
            std::cout << "\x1B[31m[[USTC_CG Error]]: " << log_content
                      << "\x1B[0m" << std::endl;
            break;
        default:;
    }
}

typedef std::function<void(Severity, char const*)> Callback;

namespace log {

void SetMinSeverity(Severity severity);
void SetCallback(Callback func);
Callback GetCallback();
void ResetCallback();

// Windows: enables or disables future log messages to be shown as
// MessageBox'es. This is the default mode. Linux: no effect, log messages are
// always printed to the console.
void EnableOutputToMessageBox(bool enable);

// Windows: enables or disables future log messages to be printed to stdout or
// stderr, depending on severity. Linux: no effect, log messages are always
// printed to the console.
void EnableOutputToConsole(bool enable);

// Windows: enables or disables future log messages to be printed using
// OutputDebugString. Linux: no effect, log messages are always printed to the
// console.
void EnableOutputToDebug(bool enable);

// Windows: sets the caption to be used by the error message boxes.
// Linux: no effect.
void SetErrorMessageCaption(const char* caption);

// Equivalent to the following sequence of calls:
// - EnableOutputToConsole(true);
// - EnableOutputToDebug(true);
// - EnableOutputToMessageBox(false);
void ConsoleApplicationMode();

void message(Severity severity, const char* fmt...);
void debug(const char* fmt...);
void info(const char* fmt...);
void warning(const char* fmt...);
void error(const char* fmt...);
void fatal(const char* fmt...);
}  // namespace log

USTC_CG_NAMESPACE_CLOSE_SCOPE
