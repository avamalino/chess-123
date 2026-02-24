#pragma once
#include <utility>
#include <sstream>
#include <vector>
#include <string>
#include <fstream>
#include <chrono>


//#ifdef Error
//#undef Error
//#endif
//
//#ifdef Debug
//#undef Debug
//#endif
//
//#ifdef Trace
//#undef Trace
//#endif

enum class LogLevel {
    Trace,
    Debug,
    Info,
    Warning,
    Error
};

struct LogEntry {
    LogLevel level;
    std::string message;
    std::chrono::system_clock::time_point timestamp;
};

class Logger {
    public:
        static void Init(const std::string& filename);
        static void Shutdown();

        static void Log(LogLevel level, const std::string& message);
        static void Clear();
        static void CopyAllToClipboard();

        static void LogTrace(const std::string& message);
        static void LogDebug(const std::string& message);
        static void LogInfo(const std::string& message);
        static void LogWarning(const std::string& message);
        static void LogError(const std::string& message);

        template<typename... Args>
        static void LogFormatted(LogLevel level, Args&&... args){
            std::ostringstream oss;
            Append(oss, std::forward<Args>(args)...); //add Append at beginning if no work
            Log(level, oss.str());
        }

        template<typename... Args>
        inline static void LogTrace(Args&&... args){
            LogFormatted(LogLevel::Trace, std::forward<Args>(args)...);
        }
        template<typename... Args>
        inline static void LogDebug(Args&&... args){
            LogFormatted(LogLevel::Debug, std::forward<Args>(args)...);
        }
        template<typename... Args>
        inline static void LogInfo(Args&&... args){
            LogFormatted(LogLevel::Info, std::forward<Args>(args)...);
        }
        template<typename... Args>
        inline static void LogWarning(Args&&... args){
            LogFormatted(LogLevel::Warning, std::forward<Args>(args)...);
        }
        template<typename... Args>
        inline static void LogError(Args&&... args){
            LogFormatted(LogLevel::Error, std::forward<Args>(args)...);
        }

        static void DrawImGui();
        static void DrawFileConsole();
        static void SetConsoleLevel(LogLevel level);

    private:
        template<typename T>
        static void Append(std::ostringstream& oss, T&& value){
            oss << std::forward<T>(value);
        }

        template<typename T, typename... Rest>
        static void Append(std::ostringstream& oss, T&& first, Rest&&... rest){
            oss << std::forward<T>(first);
            Append(oss, std::forward<Rest>(rest)...);
        }
        
        static std::vector<LogEntry> sEntries;
        static std::ofstream sFile;
        static LogLevel sConsoleLevel;
};