#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <vector>
#include <string>

namespace srvmon{

struct Server {
    std::string id;
    std::string name;
    std::string group_name;
};

struct Alarm_rule {
    bool offline;
    bool cpu;
    double cpu_threshold;
    bool memory;
    double memory_threshold;
};

struct Notice_tg {
    bool enable;
    std::string tg_token;
    std::string chat_id;
};

struct Notice_wechat {
    bool enable;
    std::string qiye_id;
    std::string agent_id;
    std::string secret;
};

// 服务端系统配置
struct Server_config
{
    std::string listen_ip;
    std::string aes_key;
    int port;
    int log_level;
    std::vector<Server> servers;
    Alarm_rule alarm_rules;
    Notice_tg notice_tg;
    Notice_wechat notice_wechat;
};

// 客户端端系统配置
struct Client_config
{
    std::string id;
    std::string server_ip;
    int port;
    std::string aes_key;
    int log_level;
};

// 客户端数据
struct Client_data
{
    std::string id;
    std::string group_name;
    std::string server_name;

    time_t accept_time;

    std::string ip;
    std::string os_name;

    int cpu_number;
    double cpu_usage;

    double mem_total;
    double mem_used;

    double swap_total;
    double swap_used;

    double disk_total;
    double disk_used;

    int process_number;

    double loadavg_one_min;
    double loadavg_five_min;
    double loadavg_fifteen_min;

    double recv_total_gb;
    double sent_total_gb;
    
    double upload_speed;
    double download_speed;
};

// 禁止拷贝构造函数
class disable_copying
{
public:
    disable_copying(){}
    virtual ~disable_copying(){}
private:
    disable_copying(const disable_copying&) = delete;
    disable_copying(disable_copying&&) = delete;
};

// 禁止赋值构造函数
class disable_assignment
{
public:
    disable_assignment(){}
    virtual ~disable_assignment(){}
private:
    disable_assignment& operator=(const disable_assignment&) = delete;
    disable_assignment& operator=(disable_assignment&&) = delete;
};

// 禁止拷贝和赋值构造函数
class disable_copying_and_assignment : public disable_copying , public disable_assignment
{
public:
    disable_copying_and_assignment(){}
    virtual ~disable_copying_and_assignment(){}
};


} // namespace

#endif