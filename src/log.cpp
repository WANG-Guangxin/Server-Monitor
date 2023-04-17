#include "log.h"

namespace srvmon{

Log::Log(const std::string& filename,const unsigned int& level = 0)
    :filename_(filename),
    stop_(false),
    level_(level)
{
    out_.open(filename_, std::ios::out | std::ios::app);
    if(!out_.is_open()){
        std::cerr << "日志文件打开失败：" << filename_ << std::endl;
        return;
    }
    log_thread_ = std::thread(&Log::thread_func,this);
}

void Log::set_log_level(const unsigned int& level)
{
    level_ = level;
}

void Log::set_file_name(const std::string& file_name)
{
    filename_ = file_name;
    if(out_.is_open())
    {
        out_.close();
    }
    out_.open(filename_, std::ios::out | std::ios::app);
    if(!out_.is_open()){
        std::cerr << "日志文件打开失败：" << filename_ << std::endl;
        return;
    }
}

std::string Log::current_time_str() const
{
    // 获取时间戳
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    // 将时间戳转换成毫秒数，然后取余数
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000; 
    // 当前时间字符串，毫秒宽度3 不足填充0 
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S.")
       << std::setfill('0') << std::setw(3) << now_ms.count();
    return ss.str();
}

std::string Log::level_to_str(LogLevel level) const{
    switch(level){
        case LogLevel::Debug:
            return " DEBUG ";
        case LogLevel::Info:
            return " INFO  ";
        case LogLevel::Warning:
            return "WARNING";
        case LogLevel::Error:
            return " ERROR ";
        default:
            return "";
    }
}


void Log::write_log(LogLevel level,const std::string& message, const int& line_number, const char* file){
    if(static_cast<unsigned int>(level) >= level_){
        std::stringstream ss;
        ss << " [" << current_time_str() << "] ";
        ss << "[" << file << ": " << std::to_string(line_number) << "] ";
        std::unique_lock<std::mutex> locker(mtx_);
        que_.push(std::make_pair(level,ss.str() + message));
        cv_.notify_one();
    }
}

void Log::thread_func()
{
    while(!stop_){
        std::unique_lock<std::mutex> locker(mtx_);
        cv_.wait_for(locker,std::chrono::milliseconds(100));

        while(!que_.empty()){
            // 写入日志文件
            auto& item = que_.front();
            out_ << '[' << level_to_str(item.first) << ']' << item.second << std::endl; 
            que_.pop();
        }
        // 刷新日志文件
        out_.flush();
    }
}

Log::~Log()
{
    // 等待日志线程退出
    stop_ = true;
    cv_.notify_all();
    if(log_thread_.joinable()){
        log_thread_.join();
    }

    // 关闭日志文件
    if(out_.is_open()){
        out_.close();
    }
}

Log& Log::Instance()
{
    static Log log_obj("./log/server.log");
    return log_obj;
}

}