#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <string>
#include <sys/time.h>
#include <cstring>
#include <sys/stat.h>
#include <stdarg.h>
#include <assert.h>
#include <fstream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <queue>
#include <ctime>
#include <iomanip> // 格式化输出
#include <sstream>
#include "definitions.h"


namespace srvmon{



enum class LogLevel{
    Debug = 0,
    Info,
    Warning,
    Error,
};

class Log : public srvmon::disable_copying_and_assignment
{
// 单例模式
private:
    Log(const std::string& filename,const unsigned int&); 
    virtual ~Log();
public:
    static Log& Instance();

public:
    void write_log(srvmon::LogLevel level, const std::string& message,const int& line_number, const char* file );
    void set_log_level(const unsigned int&);
    void set_file_name(const std::string& file_name);

private:
    void thread_func();
    std::string level_to_str(srvmon::LogLevel level) const;
    std::string current_time_str() const;
    

private:
    unsigned int level_;
    std::string filename_;
    std::ofstream out_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::thread log_thread_;
    bool stop_;
    std::queue<std::pair<LogLevel,std::string>> que_;

};


#define LOG_DEBUG(msg) srvmon::Log::Instance().write_log(srvmon::LogLevel::Debug,msg, __LINE__, __FILE__)
#define LOG_INFO(msg) srvmon::Log::Instance().write_log(srvmon::LogLevel::Info, msg,__LINE__, __FILE__)
#define LOG_WARNING(msg) srvmon::Log::Instance().write_log(srvmon::LogLevel::Warning, msg, __LINE__, __FILE__)
#define LOG_ERROR(msg) srvmon::Log::Instance().write_log(srvmon::LogLevel::Error, msg, __LINE__, __FILE__)

}

#endif