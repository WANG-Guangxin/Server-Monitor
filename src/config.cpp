#include "config.h"

namespace srvmon{

Config::Config(const std::string& config_file_name)
    :
    config_file_name_(config_file_name)
{
}

int Config::load_config(srvmon::Server_config& server_config) const
{
    std::ifstream cfg_file(config_file_name_);
    if(!cfg_file.is_open())
    {
        std::cerr << __FILE__ << ":" << __LINE__ << " " << "配置文件打开失败：" << config_file_name_ << std::endl;
        return -1;
    }

    
    std::ostringstream content;
    content << cfg_file.rdbuf();
    std::string json_str = content.str();

    json cfg_json;
    try
    {
        cfg_json = json::parse(json_str);
    }
    catch(const std::exception& e)
    {
        std::cerr << __FILE__ << ":" << __LINE__ << " " <<"Json解析异常：" << e.what() << std::endl;
        return -2;
    }

    try
    {
        server_config.listen_ip = cfg_json["listen_ip"];
        server_config.aes_key = cfg_json["aes_key"];
        server_config.port = cfg_json["port"];
        server_config.log_level = cfg_json["log_level"];

        for(auto server:cfg_json["servers"])
        {
            srvmon::Server srv;
            srv.id = server["id"];
            srv.name = server["name"];
            srv.group_name = server["group_name"];
            server_config.servers.push_back(srv);
        }

        srvmon::Alarm_rule alarm_rules;
        alarm_rules.offline = cfg_json["alarm_rules"]["offline"];
        alarm_rules.cpu = cfg_json["alarm_rules"]["cpu"];
        alarm_rules.cpu_threshold = cfg_json["alarm_rules"]["cpu_threshold"];
        alarm_rules.memory = cfg_json["alarm_rules"]["memory"];
        alarm_rules.memory_threshold = cfg_json["alarm_rules"]["memory_threshold"];
        server_config.alarm_rules = alarm_rules;

        auto notice_array = cfg_json["notice"];
        for(auto& notice : notice_array)
        {
            if(notice.find("notice_tg") != notice.end())
            {
                server_config.notice_tg.enable = notice["notice_tg"];
                server_config.notice_tg.tg_token = notice["tg_token"];
                server_config.notice_tg.chat_id = notice["chat_id"];
            }
            else if(notice.find("notice_wechat") != notice.end())
            {
                server_config.notice_wechat.enable = notice["notice_wechat"];
                server_config.notice_wechat.qiye_id = notice["qiye_id"];
                server_config.notice_wechat.agent_id = notice["agent_id"];
                server_config.notice_wechat.secret = notice["secret"];
            }
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << __FILE__ << ":" << __LINE__ << " " << "Json内容异常：" << e.what() << std::endl;
        return -3;
    }
    
    return 0;

}

int Config::load_config(srvmon::Client_config& client_config) const
{
    std::ifstream cfg_file(config_file_name_);
    if(!cfg_file.is_open())
    {
        std::cerr << __FILE__ << ":" << __LINE__ << " " << "配置文件打开失败：" << config_file_name_ << std::endl;
        return -1;
    }

    std::ostringstream content;
    content << cfg_file.rdbuf();
    std::string json_str = content.str();

    json cfg_json;
    try
    {
        cfg_json = json::parse(json_str);
    }
    catch(const std::exception& e)
    {
        std::cerr << __FILE__ << ":" << __LINE__ << " " <<"Json解析异常：" << e.what() << std::endl;
        return -2;
    }

    try
    {
        client_config.id = cfg_json["id"];
        client_config.server_ip = cfg_json["server_ip"];
        client_config.aes_key = cfg_json["aes_key"];
        client_config.port = cfg_json["port"];
        client_config.log_level = cfg_json["log_level"];
    }
    catch(const std::exception& e)
    {
        std::cerr << __FILE__ << ":" << __LINE__ << " " << "Json内容异常：" << e.what() << std::endl;
        return -3;
    }

    return 0;

}

Config::~Config()
{

}

}