///
/// @file log.cppm
module;
#include <format>
#include <print>
#include <filesystem>
#include <cstdlib>
#include <cstddef>
#include <source_location>
#include <string_view>
#include <iostream>
#include <utility>

///
/// @brief **Log system** - Colored, contextual logging with statistics
///
/// **Log Levels** (by severity):
/// | Level  | Color    | DEBUG-only | Description |
/// |--------|----------|------------|-------------|
/// | Trace  | Cyan     | ✅         | Fine-grained tracing |
/// | Debug  | Blue     | ✅         | Debug information |
/// | Info   | Green    | ❌         | General information |
/// | Warn   | Yellow   | ❌         | Warnings |
/// | Error  | Red      | ❌         | Recoverable errors |
/// | Fatal  | Magenta  | ❌         | Fatal errors (program abort) |
///
/// **Output Format**:
/// ```
/// [LEVEL] [filename:line function] | message
/// ```
///
/// Exemple of use:
/// @par Example
/// @code{.cpp}
/// import mka.graphic.log;
/// using namespace mka::graphic;
///
/// int main() {
///
///     bool is_null = true;
///     Log::trace("pointer is null ? {}.", is_null);
///		// output : "[TRACE] pointer is null ? true."
///
///     Log::trace(Log::getContext(), "pointer is null ? {}.", is_null);
///		// output : "[TRACE] main.cpp:10 main | pointer is null ? true."
///
///     Log::debug("init engine failed.");
///		// output : "[DEBUG] init engine failed."
///
///     Log::debug(Log::getContext(), "function foo(...) sucessfully reached.");
///		// output : "[DEBUG] main.cpp:16 main | function foo(...) sucessfully reached."
///
///     Log::info("my_module sucessfully load data.");
///		// output : "[INFO ] my_module sucessfully load data."
///
///     Log::warn("unused cached value.");
///		// output : "[WARN ] unused cached value."
///
///     Log::error("failed to parse string");
///		// output : "[ERROR] failed to parse string."
///
///     Log::fatal("data type mismatch.");
///		// output : "[FATAL] data type mismatch."
///		// exit immediately of the program (crash)
///
///		return 0;
/// }
/// @endcode
/// 
/// @note
/// - **Trace/Debug**: Only active when `DEBUG` is defined
/// - **Fatal**: Calls `std::abort()` immediately
/// - **Colors**: ANSI escape codes (works in most modern terminals)
///
export module mka.graphic.log;

export namespace mka::graphic {

    ///
    /// @brief Log system providing various colored log level
    ///
    class Log {
    private:
        enum class Level { Trace = 0, Debug = 1, Info = 2, Warn = 3, Error = 4, Fatal = 5 };

        static constexpr std::string_view colors[] = {"\033[0;36m", "\033[0;94m", "\033[0;32m",
                                                      "\033[0;93m", "\033[0;91m", "\033[0;95m"};
        static constexpr std::string_view reset = "\033[0m";
        static constexpr std::string_view bold = "\033[1m";

    public:
        Log() = delete;
        Log(const Log &) = delete;
        Log(Log &&) = delete;
        Log &operator=(const Log &) = delete;
        Log &operator=(Log &&) = delete;
        ~Log() = delete;

		///
        /// @name Trace/Debug Logging (DEBUG-only)
        /// @{

        ///
        /// @brief log tracinging data with context
        ///
        /// @param loc current context
        /// @param fmt formated string
        /// @param args arguments to complete the formated string
        ///
        /// @note this function print tracing info only if DEBUG macro is defined
        ///
        /// @see getContext()
        ///
		/// @par Example
        /// @code{.cpp}
        /// int main() {
        ///		int v = 42;
        ///		Log::trace(getContext(), "value is now {}.", v);
        ///		// output : "[TRACE] main.cpp:3 main | value is now 42."
        ///	}
        /// @endcode
        ///
        template <typename... Args>
        static void trace([[maybe_unused]] std::source_location loc, [[maybe_unused]] std::string_view fmt,
                          Args &&...args) {
#ifdef DEBUG
            print_colored(Level::Trace, loc, fmt, args...);
            ++traceCount;
#endif
        }

        ///
        /// @brief log debugging data with context
        ///
        /// @param loc current context
        /// @param fmt formated string
        /// @param args arguments to complete the formated string
        ///
        /// @note this function print debugging info only if DEBUG macro is defined
        ///
        /// @see getContext()
        ///
		/// @par Example
        /// @code{.cpp}
        /// int main() {
        ///		int v = 42;
        ///		Log::debug(getContext(), "value == 42 ? {}.", (v == 42));
        ///		// output : "[DEBUG] main.cpp:3 main | value == 42 ? true."
        ///	}
        /// @endcode
        ///
        template <typename... Args>
        static void debug([[maybe_unused]] std::source_location loc, [[maybe_unused]] std::string_view fmt,
                          Args &&...args) {
#ifdef DEBUG
            print_colored(Level::Debug, loc, fmt, args...);
            ++debugCount;
#endif
        }

        ///
        /// @brief log tracing data without context
        ///
        /// @param fmt formated string
        /// @param args arguments to complete the formated string
        ///
        /// @note this function print tracing info only if DEBUG macro is defined
        ///
		/// @par Example
        /// @code{.cpp}
        /// int main() {
        ///		bool v = true;
        ///		Log::trace("step 1 started ? {}.", v);
        ///		// output : "[TRACE] step 1 started ? true."
        ///	}
        /// @endcode
        ///
        template <typename... Args> static void trace([[maybe_unused]] std::string_view fmt, Args &&...args) {
#ifdef DEBUG
            print_colored(Level::Trace, {}, fmt, args...);
            ++traceCount;
#endif
        }

        ///
        /// @brief log debugging data without context
        ///
        /// @param fmt formated string
        /// @param args arguments to complete the formated string
        ///
        /// @note this function print debugging info only if DEBUG macro is defined
        ///
		/// @par Example
        /// @code{.cpp}
        /// int main() {
        ///		int v = 3;
        ///		Log::debug("foo(...) has returned {}.", v);
        ///		// output : "[DEBUG] step 1 started ? true."
        ///	}
        /// @endcode
        ///
        template <typename... Args> static void debug([[maybe_unused]] std::string_view fmt, Args &&...args) {
#ifdef DEBUG
            print_colored(Level::Debug, {}, fmt, args...);
            ++debugCount;
#endif
        }

		/// @}

		///
        /// @name Logging (Always active)
        /// @{

        ///
        /// @brief log information
        ///
        /// @param fmt formated string
        /// @param args arguments to complete the formated string
		///
		/// @par Example
        /// @code{.cpp}
        /// int main() {
        ///		Log::info("Server started on port {}", 8080);
        ///		// output : "[INFO ] Server started on port 8080"
		/// }
        /// @endcode
		///
        template <typename... Args> static void info(std::string_view fmt, Args &&...args) {
            print_colored(Level::Info, {}, fmt, args...);
            ++infoCount;
        }

        ///
        /// @brief log warning
        ///
        /// @param fmt formated string
        /// @param args arguments to complete the formated string
		///
		/// @par Example
        /// @code{.cpp}
        /// int main() {
        ///		Log::warn("Configuration file deprecated: {}", old_path);
        ///		// output : "[WARN ] Configuration file deprecated: /etc/old.conf"
		/// }
        /// @endcode
		///
        template <typename... Args> static void warn(std::string_view fmt, Args &&...args) {
            print_colored(Level::Warn, {}, fmt, args...);
            ++warnCount;
        }

        ///
        /// @brief log error
        ///
        /// @param fmt formated string
        /// @param args arguments to complete the formated string
		///
		/// @par Example
        /// @code{.cpp}
        /// int main() {
		///		const char* filename = "data.txt";
        ///		Log::error("Failed to open file: {}", filename);
        ///		// [ERROR] Failed to open file: data.txt
		/// }
        /// @endcode
		///
        template <typename... Args> static void error(std::string_view fmt, Args &&...args) {
            print_colored(Level::Error, {}, fmt, args...);
            ++errorCount;
        }

        ///
        /// @brief log fatal error
        ///
        /// @param fmt formated string
        /// @param args arguments to complete the formated string
        ///
        /// @warning this function abort the program and make imposible to resume and fix the issues
		///
        /// @par Example
        /// @code{.cpp}
        /// int main() {
        ///     Log::fatal("Cannot initialize GPU. Aborting.");
        ///     // [FATAL] Cannot initialize GPU. Aborting.
        ///     // -> std::abort() called
        /// }
        /// @endcode
		///
        template <typename... Args> static void fatal(std::string_view fmt, Args &&...args) {
            print_colored(Level::Fatal, {}, fmt, args...);
            ++fatalCount;
            std::cerr.flush();
            std::abort();
        }
		/// @}

        ///
        /// @name Statistics
        /// @{

		/// @brief Get total trace log count (DEBUG-only)
        static size_t getTraceCount() noexcept { return traceCount; }

		/// @brief Get total debug log count (DEBUG-only)
        static size_t getDebugCount() noexcept { return debugCount; }

		/// @brief Get total info log count
        static size_t getInfoCount() noexcept { return infoCount; }

		/// @brief Get total warning count
        static size_t getWarnCount() noexcept { return warnCount; }

		/// @brief Get total error count
        static size_t getErrorCount() noexcept { return errorCount; }

		/// @brief Get total fatal count
        static size_t getFatalCount() noexcept { return fatalCount; }

		/// @brief Get total logs across all levels
        static size_t getLogCount() noexcept {
            return traceCount + debugCount + infoCount + warnCount + errorCount + fatalCount;
        }

		/// @}

        ///
        /// @brief get the metadata of the function/method that call it
        ///
        /// @param loc metadata of function
        ///
        /// @return metadata given in parameter
        ///
        /// @note the aim of this function is to be called without any parameter, deducing context is automatic.
        ///
        /// @code
        /// // foo.h
        /// void foo_impl() {
        ///		auto ctx = getContext(); // { file_name="foo.h", line=3, function_name="foo_impl" }
        /// }
        /// @endcode
        ///
        static std::source_location getContext(std::source_location loc = std::source_location::current()) {
            return loc;
        }

    private:
		static inline size_t traceCount = 0; ///< @private Total trace logs (DEBUG-only)
        static inline size_t debugCount = 0; ///< @private Total debug logs (DEBUG-only)
        static inline size_t infoCount  = 0; ///< @private Total info logs
        static inline size_t warnCount  = 0; ///< @private Total warning logs
        static inline size_t errorCount = 0; ///< @private Total error logs
        static inline size_t fatalCount = 0; ///< @private Total fatal logs

        ///
        /// @brief print colored log
        ///
        /// This method print a colored log following this format :
        ///
        /// [<log level>] <file name>:<line> <function name> | <user string>
        ///
        /// @param lvl log level desired.
        /// @param loc metadata about a function (file name, function name and line).
        /// @param args data to display.
        ///
        /// @note empty loc is not displayed.
        ///
        /// @see getContext(), trace(), debug(), info(), warn(), error(), fatal()
        ///
        template <typename... Args>
        static void print_colored(Level lvl, std::source_location loc, std::string_view fmt, Args &&...args) {
            std::string_view color = colors[static_cast<size_t>(lvl)];

            std::print(std::cerr, "{}{}[{}] ", bold, color, level_name(lvl), reset);

            if constexpr (sizeof...(args) > 0) {
                std::print(std::cerr, "{}", std::vformat(fmt, std::make_format_args(args...)));
            } else {
                std::print(std::cerr, "{}", fmt);
            }

            if (loc.file_name() && *loc.file_name() != '\0') {
                std::print(std::cerr, " {:<20}:{:<4} {:<30} |",
                           std::filesystem::path(loc.file_name()).filename().string(), loc.line(),
                           std::string_view(loc.function_name()).substr(0, 30));
            }

            std::print(std::cerr, "{}\n", reset);
        }

        ///
        /// @brief transform enum Level value to string.
        ///
        /// @param lvl log level desired.
        ///
        /// @return string corresponding with enum Level value.
        ///
        /// @note used in print_colored.
        ///
        static constexpr std::string_view level_name(Level lvl) {
            using enum Level;
            switch (lvl) {
                case Trace:
                    return "TRACE";
                case Debug:
                    return "DEBUG";
                case Info:
                    return "INFO ";
                case Warn:
                    return "WARN ";
                case Error:
                    return "ERROR";
                case Fatal:
                    return "FATAL";
            }
            return "UNKWN";
        }
    };

} // namespace mka::graphic
