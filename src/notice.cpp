#include "notice.h"

namespace srvmon{

Notice::Notice(const srvmon::Server_config& server_config,
               const std::vector<srvmon::Client_data>& data_center)
    :
    alarm_rules_(server_config.alarm_rules),
    notice_tg_(server_config.notice_tg),
    notice_wechat_(server_config.notice_wechat)
{
    for(auto& data:data_center)
    {
        last_notice_time_.insert({data.id,Last_notice_time()});
    }
}

void Notice::notice_check(const std::vector<srvmon::Client_data>& vec_client_data,
                    std::unordered_map<std::string,bool>& is_online_map)
{
    static std::unordered_map<std::string,bool> last_is_online_ = is_online_map;
    if ( alarm_rules_.offline )
    {
        for(auto& data:vec_client_data)
        {
            // 上次在线，现在不在线，通知 主机离线
            if(last_is_online_[data.id] && !is_online_map[data.id])
            {
                notice_send(data,srvmon::Notification_type::Host_offline);
            }
            else if( !last_is_online_[data.id] && is_online_map[data.id])
            {// 上次离线，现在在线，通知 主机上线
                notice_send(data,srvmon::Notification_type::Host_online);
            }
        }
        last_is_online_ = is_online_map;
    }
    if ( alarm_rules_.cpu )
    {
        for(auto& data:vec_client_data)
        {
            if(data.cpu_usage/100 > alarm_rules_.cpu_threshold)
            {
                notice_send(data,srvmon::Notification_type::CPU_overrun);
            }
        }
    }
    if ( alarm_rules_.memory )
    {
        for(auto& data:vec_client_data)
        {
            if(data.mem_used/data.mem_total/100.0 > alarm_rules_.memory_threshold)
            {
                notice_send(data,srvmon::Notification_type::MEM_overrun);
            }
        }
    }
}

void Notice::notice_send(const srvmon::Client_data& data,srvmon::Notification_type&& type)
{
    
    switch(type)
    {
        case srvmon::Notification_type::Host_online:
        {
            if(notice_tg_.enable)
            {
                std::stringstream msg;
                msg << "主机已上线！\nID: " << data.id << "\nName: " << data.server_name;
                send_to_tg(notice_tg_.tg_token,notice_tg_.chat_id,msg.str());
            }
            if(notice_wechat_.enable)
            {
                std::stringstream msg;
                msg << "主机已上线！(ID: " << data.id << " Name: " << data.server_name << ")";
                send_to_wecom(msg.str(), notice_wechat_.qiye_id, notice_wechat_.agent_id, notice_wechat_.secret);
            }
            break;
        }
        case srvmon::Notification_type::Host_offline:
        {
            if(notice_tg_.enable)
            {
                std::stringstream msg;
                msg << "主机已离线！\nID: " << data.id << "\nName: " << data.server_name;
                send_to_tg(notice_tg_.tg_token,notice_tg_.chat_id,msg.str());
            }
            if(notice_wechat_.enable)
            {
                std::stringstream msg;
                msg << "主机已离线！(ID: " << data.id << " Name: " << data.server_name << ")";
                send_to_wecom(msg.str(), notice_wechat_.qiye_id, notice_wechat_.agent_id, notice_wechat_.secret);
            }
            break;
        }
        case srvmon::Notification_type::CPU_overrun:
        {
            auto now = std::chrono::system_clock::now();
            if( std::chrono::system_clock::to_time_t(now) - last_notice_time_[data.id].cpu > 60)
            {
                last_notice_time_[data.id].cpu = std::chrono::system_clock::to_time_t(now);
                if(notice_tg_.enable)
                {
                    std::stringstream msg;
                    msg << "主机CPU占用率过高！\nID: " << data.id << "\nName: " << data.server_name
                        << "\nCPU使用率：" << data.cpu_usage << "%";
                    send_to_tg(notice_tg_.tg_token,notice_tg_.chat_id,msg.str());
                }
                if(notice_wechat_.enable)
                {
                    std::stringstream msg;
                    msg << "主机CPU占用率过高！(ID: " << data.id << " Name: " << data.server_name 
                        << " CPU使用率：" << data.cpu_usage << "%)";
                    send_to_wecom(msg.str(), notice_wechat_.qiye_id, notice_wechat_.agent_id, notice_wechat_.secret);
                }
            }
            break;
        }
        case srvmon::Notification_type::MEM_overrun:
        {
            auto now = std::chrono::system_clock::now();
            if( std::chrono::system_clock::to_time_t(now) - last_notice_time_[data.id].mem > 60)
            {
                last_notice_time_[data.id].mem = std::chrono::system_clock::to_time_t(now);
                if(notice_tg_.enable)
                {
                    std::stringstream msg;
                    msg << "主机内存占用率过高！\nID: " << data.id << "\nName: " << data.server_name
                        << "\n内存使用率：" << data.mem_used/data.mem_total << "%";
                    send_to_tg(notice_tg_.tg_token,notice_tg_.chat_id,msg.str());
                }
                if(notice_wechat_.enable)
                {
                    std::stringstream msg;
                    msg << "主机内存占用率过高！(ID: " << data.id << " Name: " << data.server_name 
                        << " 内存使用率：" << data.mem_used/data.mem_total << "%)";
                    send_to_wecom(msg.str(), notice_wechat_.qiye_id, notice_wechat_.agent_id, notice_wechat_.secret);
                }
            }
            break;
        }
        default:
        {

        }
    }
}

size_t Notice::write_callback(char* ptr, size_t size, size_t nmemb, std::string* data)
{
    data->append(ptr, size * nmemb);
    return size*nmemb;
}

int Notice::send_to_wecom(const std::string& msg,
                          const std::string& wecom_cid,
                          const std::string& wecom_aid,
                          const std::string& wecom_secret,
                          const std::string& wecom_touid)
{
    std::string get_token_url = 
        "https://qyapi.weixin.qq.com/cgi-bin/gettoken?corpid=" + wecom_cid +
        "&corpsecret=" + wecom_secret;

    CURL* curl = curl_easy_init();
    if(!curl)
    {
        LOG_ERROR("企业微信通知失败：curl 初始化失败！");
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, get_token_url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

    std::string response;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    LOG_DEBUG("企业微信Token响应：");
    LOG_DEBUG(response);
    if (res != CURLE_OK)
    {
        curl_easy_cleanup(curl);
        LOG_ERROR(std::string("企业微信通知失败：获取Token失败！") + curl_easy_strerror(res));
        return -1;
    }

    curl_easy_cleanup(curl);

    json json_response = json::parse(response);
    
    std::string access_token = json_response.value("access_token", "");

    if(access_token.empty())
    {
        LOG_ERROR("企业微信通知失败：Token解析失败！");
        return -1;
    }

    std::string send_msg_url = 
                "https://qyapi.weixin.qq.com/cgi-bin/message/send?access_token=" +
                access_token;
    
    curl = curl_easy_init();
    if (!curl)
    {
        LOG_ERROR("企业微信通知失败：curl 初始化失败！");
        return -1;
    }

    json data = 
    {
        {"touser", wecom_touid},
        {"agentid", wecom_aid},
        {"msgtype", "text"},
        {"text", {{"content", msg}}},
        {"duplicate_check_interval", 600}
    };

    // 处理 json 字符串 将换行变成空格
    std::string json_str = data.dump(0).c_str();
    std::replace(json_str.begin(),json_str.end(), '\n', ' ');
    LOG_DEBUG( std::string("企业微信通知Json：") + json_str);

    curl_easy_setopt(curl, CURLOPT_URL, send_msg_url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str.c_str());

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    res = curl_easy_perform(curl);

    LOG_DEBUG( std::string("企业微信响应Json：") + response);

    if (res != CURLE_OK)
    {
        curl_easy_cleanup(curl);
        LOG_ERROR("企业微信通知失败：消息发送失败！");
        return -1;
    }

    curl_easy_cleanup(curl);
    return 0;
}

int Notice::send_to_tg(const std::string& bot_token, const std::string& chat_id, const std::string& msg)
{
    std::stringstream ss_api_url;
    ss_api_url << "https://api.telegram.org/" << bot_token << "/sendMessage";
    json data = 
    {
        {"chat_id", chat_id},
        {"text", msg},
        {"parse_mode", "HTML"}
    };
    std::string json_str = data.dump(0).c_str();
    LOG_DEBUG("TG通知: ");
    LOG_DEBUG(json_str);

    std::string api_url = ss_api_url.str();

    // 创建 curl 句柄
    CURL* curl_handle = curl_easy_init();

    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    if(curl_handle)
    {
        // 设置请求的 URL 地址
        curl_easy_setopt(curl_handle, CURLOPT_URL, api_url.c_str());
        // 关闭 SSL 主机名验证
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0);
        // 关闭 SSL 证书验证
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl_handle, CURLOPT_POST, 1L);
        curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, json_str.c_str());
        curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
        // 执行请求
        CURLcode res = curl_easy_perform(curl_handle);
        // 判断请求是否成功
        if (res != CURLE_OK)
        {
            LOG_ERROR( curl_easy_strerror(res));
            return 2;
        }
        else
        {
            return 0;
        }
    }
    else{
        return 1;
    }
}

Notice::~Notice()
{

}


} // namespace