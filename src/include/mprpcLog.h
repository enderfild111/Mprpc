
#include <string>
#include "unablecopy.h"
#include "safetyQueue.h"
#include <source_location>
#include <thread>
#include <ctime>
#include <chrono>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <condition_variable>
#include <atomic>

namespace fs = std::filesystem;

#define LOG(level) \
    LogStream(LogLevel::level,std::source_location::current())

enum class LogLevel
{
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

struct LogMsg
{
    LogLevel level;
    std::string text;
    std::source_location location;
    std::chrono::system_clock::time_point timestamp;

    std::string level_string() const
    {
        switch (level)
        {
        case LogLevel::INFO:
            return "INFO";
            break;
        case LogLevel::DEBUG:
            return "DEBUG";
            break;
        case LogLevel::WARNING:
            return "WARNING";
            break;
        case LogLevel::ERROR:
            return "ERROR";
            break;
        default:
            return "";
        }
    }
    std::string format() const
    {
        auto time = std::chrono::system_clock::to_time_t(timestamp);
        char timebuf[32];
        std::string file_name = location.file_name() != nullptr ? location.file_name(): "Missing file name";
        std::string func_name = location.function_name() != nullptr ? location.function_name() : "Missing function name";

        std::strftime(timebuf,sizeof(timebuf),"%Y-%m-%d %H:%M:%S",std::localtime(&time));
        return "[" + std::string(timebuf) + "]" + "[" + level_string() + "]" + file_name + ":" 
        + std::to_string(location.line()) + ":" + func_name + "-" + text;
    }
};

class MprpcLogger : public UnableCopy
{
public:
    static MprpcLogger& getInstance()
    {
        static MprpcLogger Instance;
        return Instance;
    }
    // 日志入队接口
    void push(LogLevel level, std::source_location loc, std::string msg) 
    {
        if(level < min_level_.load(std::memory_order_relaxed))
        {
            return;
        }
        queue_.push(LogMsg{
            .level = level,
            .text = std::move(msg),
            .location = loc,
            .timestamp = std::chrono::system_clock::now()
        });
    }
    LogLevel string_to_Level(const std::string &level) const
    {
        if (level == "INFO")
        {
            return LogLevel::INFO;
        }
        else if (level == "DEBUG")
        {
            return LogLevel::DEBUG;
        }
        else if (level == "WARNING")
        {
            return LogLevel::WARNING;
        }
        else if (level == "ERROR")
        {
            return LogLevel::ERROR;
        }
        else
        {
            return LogLevel::INFO;
        }
    }
    void openFile()
    {
        if (!ensure_parent_dir_exists(filePath_))
        {
            std::cerr << "Failed to create log directory for: " << filePath_ << std::endl;
            log_enable_ = false;
            return;
        }
        if (log_file_.is_open())
        {
            log_enable_ = true;
            return;
        }
        log_file_.open(filePath_, std::ios::app);
        if(log_file_.is_open())
        {
            log_enable_ = true;
            return;
        }
        else
        {
            log_enable_ = false;
            std::cerr << "Failed to open log file for" << filePath_ << std::endl;
            return;
        }
    }

    void set_minLogLevel(const LogLevel& level)
    {
        min_level_.store(level,std::memory_order_relaxed);
    }

    LogLevel get_minLogLevel() const
    {
        return min_level_.load(std::memory_order_relaxed);
    }

    ~MprpcLogger()
    {
        worker_.request_stop();
        if(worker_.joinable())
        {
           worker_.join();
        }
        if(log_file_.is_open())
        {
            log_file_.flush();
        }
    }
private:
    SafeQueue<LogMsg> queue_;  // 日志队列
    std::ofstream log_file_;   // 日志文件流
    std::string filePath_;     // 日志路径
    bool log_enable_;          // 日志文件是否可写
    std::string current_data_; // 当前时间 YYYY-MM-DD
    std::string log_base_name_;// 日志基础名
    std::atomic<LogLevel> min_level_{LogLevel::DEBUG};
    std::jthread worker_;      // 工作线程(写日志)

    bool ensure_parent_dir_exists(const std::string& filePath) {
        std::error_code ec;
        fs::create_directories(fs::path(filePath).parent_path(), ec);
        return !ec;  // 有错误返回 false，成功（包括目录已存在）返回 true
    }

    std::string get_currentdata(const std::chrono::system_clock::time_point& tp) const
    {
        auto time = std::chrono::system_clock::to_time_t(tp);
        std::tm *tm  =  std::localtime(&time);
        std::ostringstream oss;
        oss << std::put_time(tm,"%Y-%m-%d");
        return oss.str();
    }

    void rotate_ifneeded(const std::chrono::system_clock::time_point& msgtp)
    {
        std::string currTime = get_currentdata(msgtp);
        if(currTime == current_data_)
        {
            return;
        }
        if(log_file_.is_open())
        {
            log_file_.close();
        }

        filePath_ = "Log/" + log_base_name_ + currTime + ".log";
        current_data_ = currTime;
        openFile();
    }
    MprpcLogger()
    : worker_([this](std::stop_token stop_token){work(stop_token);}) 
    , log_base_name_("Mprpc_")
    , log_enable_(false)
    {
        current_data_ = get_currentdata(std::chrono::system_clock::now());
        filePath_ = "Log/" + log_base_name_ + current_data_ + ".log";
        openFile();
    }

    void work(std::stop_token stop_token)
    {
        while(!stop_token.stop_requested())
        {
            auto logmsg = queue_.try_pop();

            if(logmsg.has_value())
            {
                rotate_ifneeded(logmsg->timestamp);
                if(log_enable_ && log_file_.is_open())
                {
                    log_file_ << logmsg->format() << std::endl;
                }
                else
                {
                    std::cerr << logmsg->format() << std::endl;
                }
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
        while (auto logmsg = queue_.try_pop())
        {
            rotate_ifneeded(logmsg->timestamp);
            if (log_enable_ && log_file_.is_open())
            {
                log_file_ << logmsg->format() << std::endl;
            }
            else
            {
                std::cerr << logmsg->format() << std::endl;
            }
        }

        if(log_file_.is_open())
        {
            log_file_.flush();
        }
    }
};



class LogStream
{
public:
    LogStream(LogLevel level,std::source_location loc)
    : level_(level)
    , location_(loc)
    {

    }
    template<typename T>
    LogStream& operator<<(const T& val)
    {
        oss_ << val;
        return *this;
    }
    ~LogStream()
    {
        MprpcLogger::getInstance().push(level_,location_,oss_.str());
    }
private:
    LogLevel level_;
    std::source_location location_;
    std::ostringstream oss_;
};