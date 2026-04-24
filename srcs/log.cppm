module;
#include <format>
#include <print>
#include <filesystem>
#include <cstdlib>
#include <cstddef>
#include <source_location>
#include <string_view>
#include <iostream>
#include <utility>  // std::forward

export module mka.graphic.log;

export namespace mka::graphic {

class Log {
private:
    enum class Level { Trace = 0, Debug = 1, Info = 2, Warn = 3, Error = 4, Fatal = 5 };

    static constexpr std::string_view colors[] = {
        "\033[0;36m", "\033[0;94m", "\033[0;32m", "\033[0;93m", 
        "\033[0;91m", "\033[0;95m"
    };
    static constexpr std::string_view reset = "\033[0m";
    static constexpr std::string_view bold = "\033[1m";

public:
    Log() = delete; Log(const Log&) = delete; Log(Log&&) = delete;
    Log& operator=(const Log&) = delete; Log& operator=(Log&&) = delete;
    ~Log() = delete;

#ifdef DEBUG
    template<typename... Args>
    static void trace(std::string_view fmt, Args&&... args) {
        print_colored(Level::Trace, std::source_location::current(), fmt, args...);
        ++traceCount;
    }

    template<typename... Args>
    static void debug(std::string_view fmt, Args&&... args) {
        print_colored(Level::Debug, std::source_location::current(), fmt, args...);
        ++debugCount;
    }
#else
    template<typename... Args>
    static void trace(std::string_view, Args&&..., std::source_location) {}
    template<typename... Args>
    static void debug(std::string_view, Args&&..., std::source_location) {}
#endif

    template<typename... Args>
    static void info(std::string_view fmt, Args&&... args) {
        print_colored(Level::Info, {}, fmt, args...);
        ++infoCount;
    }

    template<typename... Args>
    static void warn(std::string_view fmt, Args&&... args) {
        print_colored(Level::Warn, {}, fmt, args...);
        ++warnCount;
    }

    template<typename... Args>
    static void error(std::string_view fmt, Args&&... args) {
        print_colored(Level::Error, {}, fmt, args...);
        ++errorCount;
    }

    template<typename... Args>
    static void fatal(std::string_view fmt, Args&&... args) {
        print_colored(Level::Fatal, {}, fmt, args...);
        ++fatalCount;
        std::cerr.flush();
        std::abort();
    }

    // Getters
    static size_t getTraceCount() noexcept { return traceCount; }
    static size_t getDebugCount() noexcept { return debugCount; }
    static size_t getInfoCount() noexcept { return infoCount; }
    static size_t getWarnCount() noexcept { return warnCount; }
    static size_t getErrorCount() noexcept { return errorCount; }
    static size_t getFatalCount() noexcept { return fatalCount; }
    static size_t getLogCount() noexcept {
        return traceCount + debugCount + infoCount + warnCount + errorCount + fatalCount;
    }

private:
    static inline size_t traceCount = 0, debugCount = 0, infoCount = 0;
    static inline size_t warnCount = 0, errorCount = 0, fatalCount = 0;

    // ✅ FIX : Pas de forward, passage direct aux format_args
    template<typename... Args>
    static void print_colored(Level lvl, std::source_location loc, std::string_view fmt, Args&&... args) {
        std::string_view color = colors[static_cast<size_t>(lvl)];
        
        if constexpr (sizeof...(args) > 0) {
            std::print(std::cerr, "{}{}[{}] {}{}", 
                bold, color, level_name(lvl), reset,
                std::vformat(fmt, std::make_format_args(args...)));
        } else {
            std::print(std::cerr, "{}{}[{}] {}{}", 
                bold, color, level_name(lvl), reset, fmt);
        }

        if (loc.file_name()) {
            std::print(std::cerr, " {:<20}:{:<4} {:<30} |", 
                std::filesystem::path(loc.file_name()).filename().string(),
                loc.line(),
                std::string_view(loc.function_name()).substr(0, 30));
            std::print(std::cerr, " {}{}{}", color, color, reset);
        }
        
        std::print(std::cerr, "\n");
    }
    
    static constexpr std::string_view level_name(Level lvl) {
        using enum Level;
        switch(lvl) {
            case Trace: return "TRACE";
            case Debug: return "DEBUG";
            case Info:  return "INFO ";
            case Warn:  return "WARN ";
            case Error: return "ERROR";
            case Fatal: return "FATAL";
        }
        return "UNKWN";
    }
};

} // namespace mka::graphic
