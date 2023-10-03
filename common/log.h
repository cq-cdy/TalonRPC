#ifndef talon_COMMON_LOG_H
#define talon_COMMON_LOG_H

#include <string>
#include <queue>
#include <memory>
#include "config.h"
#include "mutex"
namespace talon {


    template<typename... Args>
    std::string formatString(const char* str, Args&&... args) {

        int size = snprintf(nullptr, 0, str, args...);

        std::string result;
        if (size > 0) {
            result.resize(size);
            snprintf(&result[0], size + 1, str, args...);
        }

        return result;
    }


#define DEBUGLOG(str, ...) \
  if (talon::Logger::GetGlobalLogger()->getLogLevel() <= talon::Debug) \
  { \
    talon::Logger::GetGlobalLogger()->pushLog(( talon::LogEvent(talon::LogLevel::Debug)).toString() \
      + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + talon::formatString(str, ##__VA_ARGS__) + "\n");\
    talon::Logger::GetGlobalLogger()->log();                                                                                \
  } \


#define INFOLOG(str, ...) \
  if (talon::Logger::GetGlobalLogger()->getLogLevel() <= talon::Info) \
  { \
  talon::Logger::GetGlobalLogger()->pushLog(( talon::LogEvent(talon::LogLevel::Info)).toString() \
    + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + talon::formatString(str, ##__VA_ARGS__) + "\n");\
  talon::Logger::GetGlobalLogger()->log();                                                                      \
  } \

#define ERRORLOG(str, ...) \
  if (talon::Logger::GetGlobalLogger()->getLogLevel() <= talon::Error) \
  { \
    talon::Logger::GetGlobalLogger()->pushLog((talon::LogEvent(talon::LogLevel::Error)).toString() \
      + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + talon::formatString(str, ##__VA_ARGS__) + "\n");\
    talon::Logger::GetGlobalLogger()->log();                                                                                 \
  } \



    enum LogLevel {
        Unknown = 0,
        Debug = 1,
        Info = 2,
        Error = 3
    };


    std::string LogLevelToString(LogLevel level);

    LogLevel StringToLogLevel(const std::string& log_level);


    class Logger {
    public:
        typedef std::shared_ptr<Logger> s_ptr;

        Logger(LogLevel level) : m_set_level(level) {}

        void pushLog(const std::string& msg);

        void log();

        LogLevel getLogLevel() const {
            return m_set_level;
        }

    public:
        static Logger* GetGlobalLogger();

        static void InitGlobalLogger();

    private:
        LogLevel m_set_level;
        std::queue<std::string> m_buffer;

        std::mutex m_mutex;

    };


    class LogEvent {
    public:

        LogEvent(LogLevel level) : m_level(level) {}

        std::string getFileName() const {
            return m_file_name;
        }

        LogLevel getLogLevel() const {
            return m_level;
        }

        std::string toString();


    private:
        std::string m_file_name;  // 文件名
        int32_t m_file_line{};  // 行号
        int32_t m_pid{};  // 进程号
        int32_t m_thread_id{};  // 线程号

        LogLevel m_level  = LogLevel::Info;     //日志级别

    };

}

#endif