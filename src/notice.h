#ifndef NOTICE_H
#define NOTICE_H

#include <unordered_map>
#include <chrono>
#include <ctime>
#include <vector>
#include <curl/curl.h>
#include <iostream>
#include <string>
#include <sstream>

#include "nlohmann/json.hpp"
#include "definitions.h"
#include "log.h"
#include "utils.h"

namespace srvmon{

using json = nlohmann::json;

struct Last_notice_time{
    time_t cpu;
    time_t mem;
    Last_notice_time()
        :
        cpu(0),
        mem(0)
    {}
};

enum class Notification_type
{
    Host_online = 0,
    Host_offline,
    CPU_overrun,
    MEM_overrun,
};

class Notice : public disable_copying_and_assignment
{
public:
    Notice(const srvmon::Server_config&,const std::vector<srvmon::Client_data>&);
    virtual ~Notice();
    void notice_check(const std::vector<srvmon::Client_data>&,
                    std::unordered_map<std::string,bool>&);

private:
    void notice_send(const srvmon::Client_data&,srvmon::Notification_type&&);
    int send_to_tg(const std::string& bot_token, 
                   const std::string& chat_id, 
                   const std::string& msg);
    int send_to_wecom(const std::string& text,
                    const std::string& wecom_cid,
                    const std::string& wecom_aid,
                    const std::string& wecom_secret,
                    const std::string& wecom_touid = "@all");
    static size_t write_callback(char* ptr, size_t size, size_t nmemb, std::string* data);

private:
    std::unordered_map<std::string,Last_notice_time> last_notice_time_;
    srvmon::Alarm_rule alarm_rules_;
    srvmon::Notice_tg notice_tg_;
    srvmon::Notice_wechat notice_wechat_;
};


} // namespace

#endif