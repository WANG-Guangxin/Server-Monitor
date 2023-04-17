#include "server.h"

namespace srvmon{

Server_monitor::Server_monitor(const std::string& config_file_name)
    :
    config_(new Config(config_file_name)),
    stop_(false)
{
    if(config_->load_config(server_config_))
    {
        exit(-23);
    }
    srvmon::Log::Instance().set_log_level(server_config_.log_level);
    for(size_t i = 0; i < server_config_.servers.size(); i++)
    {
        srvmon::Client_data client_data;
        client_data.id = server_config_.servers.at(i).id;
        client_data.group_name = server_config_.servers.at(i).group_name;
        client_data.server_name = server_config_.servers.at(i).name;
        client_data.cpu_number = 0;
        client_data.process_number = 0;

        data_center_.push_back(client_data);
        id_to_idx_.insert({server_config_.servers.at(i).id,i});
    }
    notice_ = std::make_shared<srvmon::Notice>(server_config_, data_center_);
    console_ = std::make_unique<srvmon::Console>(data_center_, notice_);
}

void Server_monitor::run()
{
    std::thread print_thread(
        [&](){
            while(true)
            {
                console_->print(data_center_);
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
        }
    );
    std::thread input_thread(
        [&](){
            console_->check_input();
        }
    );
    print_thread.detach();
    input_thread.detach();
    accept_data();
}

int Server_monitor::accept_data()
{
    // 创建套接字
    int server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if( server_socket == -1)
    {
        LOG_ERROR("创建Socket失败！");
        return -1;
    }

    // 设置服务器地址
    sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(server_config_.listen_ip.c_str());
    server_address.sin_port = htons(server_config_.port);
    std::stringstream log_str;
    log_str << "UDP地址：" << server_config_.listen_ip << ":" << std::to_string(server_config_.port);
    LOG_INFO(log_str.str());

    if(bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) == -1)
    {
        LOG_ERROR("绑定 IP 和端口失败！");
        close(server_socket);
        return -1;
    }

    // 接收数据
    while(!stop_)
    {
        sockaddr_in client_address;
        memset(&client_address, 0, sizeof(client_address));
        socklen_t client_length = sizeof(client_address);
        char buffer[1024] = {0};
        ssize_t received_bytes = recvfrom(server_socket, buffer, sizeof(buffer), 0,
                                         (struct sockaddr*) &client_address, &client_length);
        if( received_bytes == -1)
        {
            LOG_ERROR("数据接收失败！");
            continue;
        }
        std::stringstream log_str;
        log_str << "接收" << received_bytes << "字节数据。";
        LOG_INFO(log_str.str());

        // 数据解密
        std::string data_str(buffer,received_bytes);
        std::string json_str = srvmon::aes_decrypt(data_str,server_config_.aes_key);
        log_str.str("");
        log_str << "解密后的Json: " <<  json_str ;
        LOG_DEBUG(log_str.str());

        // json 解析
        json json_obj;
        try
        {
            json_obj = json::parse(json_str);
        }
        catch(std::exception& e)
        {
            log_str.str("");
            log_str << "Json 解析失败：" << e.what();
            LOG_ERROR(log_str.str());
            continue;
        }

        std::string id = json_obj["id"];
        if( id_to_idx_.count(id) )
        {
            int idx = id_to_idx_[id];
            // 更新数据
            auto now = std::chrono::system_clock::now();
            data_center_[idx].accept_time = std::chrono::system_clock::to_time_t(now);
            data_center_[idx].ip = inet_ntoa(client_address.sin_addr);
            data_center_[idx].os_name = json_obj["os_name"];
            data_center_[idx].cpu_number = json_obj["cpu_number"];
            data_center_[idx].cpu_usage = json_obj["cpu_usage"];
            data_center_[idx].mem_total = json_obj["mem_total"];
            data_center_[idx].mem_used = json_obj["mem_used"];
            data_center_[idx].swap_total = json_obj["swap_total"];
            data_center_[idx].swap_used = json_obj["swap_used"];
            data_center_[idx].disk_total = json_obj["disk_total"];
            data_center_[idx].disk_used = json_obj["disk_used"];
            data_center_[idx].process_number = json_obj["process_number"];
            data_center_[idx].loadavg_one_min = json_obj["loadavg_one_min"];
            data_center_[idx].loadavg_five_min = json_obj["loadavg_five_min"];
            data_center_[idx].loadavg_fifteen_min = json_obj["loadavg_fifteen_min"];
            data_center_[idx].recv_total_gb = json_obj["recv_total_gb"];
            data_center_[idx].sent_total_gb = json_obj["sent_total_gb"];
            data_center_[idx].upload_speed = json_obj["upload_speed"];
            data_center_[idx].download_speed = json_obj["download_speed"];
            log_str.str("");
            log_str << "更新数据成功：" << id << " " << inet_ntoa(client_address.sin_addr);
            LOG_INFO(log_str.str());
        }

    }

    return 0;
}

Server_monitor::~Server_monitor()
{

}


} // namespace