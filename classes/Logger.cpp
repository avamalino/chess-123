#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <csignal>
#include <exception>
#include "Logger.h"
#include "../imgui/imgui.h"

static const char* ToString(LogLevel level){
    switch(level){
        case LogLevel::Trace:   return "TRACE";
        case LogLevel::Debug:   return "DEBUG";
        case LogLevel::Info:    return "INFO";
        case LogLevel::Warning: return "WARNING";
        case LogLevel::Error:   return "ERROR";
        default:                return "UNKNOWN";
    }
};

std::vector<LogEntry> Logger::sEntries;
std::ofstream Logger::sFile;
LogLevel Logger::sConsoleLevel = LogLevel::Trace;

void Logger::Init(const std::string& filename){
    // Open in append mode so existing logs aren't overwritten during dev
    sFile.open(filename, std::ios::out | std::ios::app);
    if (sFile.is_open()){
        sFile << "[Logger] Initialized. Current working directory: " << std::filesystem::current_path().string() << "\n";
        sFile.flush();
    } else {
        std::cerr << "Logger: failed to open log file '" << filename << "' in current path '" << std::filesystem::current_path().string() << "'\n";
    }

    // Register terminate and signal handlers so we get a chance to flush logs
    std::set_terminate([](){
        try {
            std::exception_ptr ep = std::current_exception();
            if (ep) std::rethrow_exception(ep);
        } catch (const std::exception& e){
            if (sFile.is_open()){
                sFile << "[Logger] terminate: std::exception: " << e.what() << "\n";
                sFile.flush();
            }
            std::cerr << "terminate: std::exception: " << e.what() << "\n";
        } catch (...) {
            if (sFile.is_open()){
                sFile << "[Logger] terminate: unknown exception\n";
                sFile.flush();
            }
            std::cerr << "terminate: unknown exception\n";
        }
        // restore default and abort
        std::abort();
    });

    auto signalHandler = [](int sig){
        if (sFile.is_open()){
            sFile << "[Logger] signal caught: " << sig << "\n";
            sFile.flush();
        }
        std::cerr << "signal caught: " << sig << "\n";
        // re-raise default handler
        std::signal(sig, SIG_DFL);
        std::raise(sig);
    };

    std::signal(SIGABRT, signalHandler);
    std::signal(SIGSEGV, signalHandler);
}

void Logger::Shutdown(){
    if (sFile.is_open()){
        sFile.close();
    }
}

static std::string ReadFileToString(const std::string& filename){
    std::ifstream file(filename);
    if (!file.is_open()){
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

static std::string FormatTime(std::chrono::system_clock::time_point tp){
    std::time_t t = std::chrono::system_clock::to_time_t(tp);
    std::tm tm;

    #if defined(_WIN32)
        localtime_s(&tm, &t);
    #else
        localtime_r(&t, &tm);
    #endif

    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &tm);
    return buffer;
}

void Logger::Log(LogLevel level, const std::string& message){
    LogEntry entry;
    entry.level = level;
    entry.message = message;
    entry.timestamp = std::chrono::system_clock::now();

    sEntries.push_back(entry);

    if (sFile.is_open()){
        sFile << "[" << FormatTime(entry.timestamp) << "][" <<
        ToString(level) << "]" << message << "\n";
        sFile.flush();
    }
}

void Logger::LogTrace(const std::string& msg){ Log(LogLevel::Trace, msg); }
void Logger::LogDebug(const std::string& msg){ Log(LogLevel::Debug, msg); }
void Logger::LogInfo(const std::string& msg){ Log(LogLevel::Info, msg); }
void Logger::LogWarning(const std::string& msg){ Log(LogLevel::Warning, msg); }
void Logger::LogError(const std::string& msg){ Log(LogLevel::Error, msg); }

void Logger::SetConsoleLevel(LogLevel level){
    sConsoleLevel = level;
}

void Logger::DrawImGui(){
    ImGui::Begin("Log Console");

    static int levelIndex = 0;
    const char* levels[] = {"Trace", "Debug", "Info", "Warning", "Error"};

    if (ImGui::Combo("Console Level", &levelIndex, levels, IM_ARRAYSIZE(levels))){
        sConsoleLevel = static_cast<LogLevel>(levelIndex);
    }

    ImGui::Separator();

    ImGui::BeginChild("Scrolling");
    for (const auto& entry: sEntries){
        if ((int)entry.level < (int)sConsoleLevel){
            continue;
        }
            ImVec4 color;
            switch (entry.level){
                case LogLevel::Trace:   color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); break;
                case LogLevel::Debug:   color = ImVec4(0.5f, 0.5f, 1.0f, 1.0f); break;
                case LogLevel::Info:    color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); break;
                case LogLevel::Warning: color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); break;
                case LogLevel::Error:   color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); break;
            }

            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::Text("[%s] [%s] %s", FormatTime(entry.timestamp).c_str(), ToString(entry.level), entry.message.c_str());
            ImGui::PopStyleColor();
        
    }
    ImGui::EndChild();
    ImGui::End();
}

void Logger::DrawFileConsole(){
    static bool scrollToBottom = true;
    ImGui::Begin("File Log Console");

    std::string logText = ReadFileToString("engine.log");

    ImGui::BeginChild("FileScroling", ImVec2(0,0), true, ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::TextUnformatted(logText.c_str());

    if (scrollToBottom){
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();
    ImGui::Checkbox("Auto-scroll", &scrollToBottom);
    ImGui::End();
}

void Logger::Clear(){
    sEntries.clear();
}

void Logger::CopyAllToClipboard(){
    ImGui::LogToClipboard();
    for (const auto& entry : sEntries)
    {
        ImGui::LogText("[%s] %s\n",
            ToString(entry.level),
            entry.message.c_str());
    }
    ImGui::LogFinish();
}