#include "client.h"

namespace srvmon{

Client::Client(const std::string& config_file_name)
    :
    stop_(false),
    ptr_config_(new Config(config_file_name))
{
    if(ptr_config_->load_config(client_config_))
    {
        std::cerr << "配置加载失败！" << std::endl;
        exit(-33);
    }
    srvmon::Log::Instance().set_log_level(client_config_.log_level);
    srvmon::Log::Instance().set_file_name("./log/client.log");
    client_data_.id = client_config_.id;
    client_data_.group_name = "";
    client_data_.server_name = "";
    client_data_.accept_time = 0;
    client_data_.ip = "";
    if( get_cpu_number(client_data_.cpu_number) )
    {
        client_data_.cpu_number = -1;
    }
    if( get_os_name(client_data_.os_name))
    {
        client_data_.os_name = "";
    }   
}

void Client::get_client_data()
{
    if( get_load_avg(client_data_.loadavg_one_min,
                     client_data_.loadavg_five_min,
                     client_data_.loadavg_fifteen_min))
    {
        client_data_.loadavg_one_min = -1.0;
        client_data_.loadavg_five_min = -1.0;
        client_data_.loadavg_fifteen_min = -1.0;
    }

    if( get_cpu_usage(client_data_.cpu_usage))
    {
        client_data_.cpu_usage = -1.0;
    }

    if( get_mem_info(client_data_.mem_total, client_data_.mem_used,
                     client_data_.swap_total, client_data_.swap_used))
    {
        client_data_.mem_total = -1.0;
        client_data_.mem_used = -1.0;
        client_data_.swap_total = -1.0;
        client_data_.swap_used = -1.0;
    }

    if( get_total_network_usage(client_data_.recv_total_gb, client_data_.sent_total_gb))
    {
        client_data_.recv_total_gb = -1.0;
        client_data_.sent_total_gb = -1.0;
    }

    if( get_net_speed(client_data_.upload_speed, client_data_.download_speed))
    {
        client_data_.upload_speed = -1.0;
        client_data_.download_speed = -1.0;
    }

    if( get_disk_info(client_data_.disk_total, client_data_.disk_used))
    {
        client_data_.disk_total = -1.0;
        client_data_.disk_used = -1.0;
    }

    if( get_process_number(client_data_.process_number))
    {
        client_data_.process_number = -1;
    }

}

int Client::get_load_avg(double& one_min, double& five_min, double& fifteen_min)
{
    std::ifstream file("/proc/loadavg");
    if( !file.is_open() )
    {
        LOG_DEBUG("/proc/loadavg 打开失败！");
        return -1;
    }
    std::string line;
    std::getline(file, line);
    std::istringstream iss(line);
    iss >> one_min >> five_min >> fifteen_min;

    std::stringstream log_str;
    log_str << "获取到系统负载：" << one_min << "/" << five_min << "/" << fifteen_min;
    LOG_DEBUG(log_str.str());

    return 0;
}
int Client::get_cpu_number(int& cpu_number)
{
    std::ifstream file("/proc/cpuinfo");
    if( !file.is_open() )
    {
        LOG_DEBUG("/proc/cpuinfo 打开失败！");
        return -1;
    }
    std::string line;
    cpu_number = 0;
    while (std::getline(file, line))
    {
        if (line.find("processor") == 0)
        {
            cpu_number++;
        }
    }

    std::stringstream log_str;
    log_str << "获取到CPU数量：" << cpu_number ;
    LOG_DEBUG(log_str.str());

    return 0;
}
int Client::get_cpu_usage(double& cpu_usage)
{
    static unsigned long long prev_total_ticks = 0;
    static unsigned long long prev_idle_ticks = 0;

    std::ifstream stat_file("/proc/stat");
    if( !stat_file.is_open() )
    {
        LOG_DEBUG("/proc/stat 打开失败！");
        return -1;
    }
    std::string line;
    std::getline(stat_file, line);
    std::istringstream iss(line);
    std::string cpu;

    unsigned long long user_ticks, nice_ticks, system_ticks, idle_ticks, 
                        io_wait_ticks, irq_ticks, soft_irq_ticks;
    iss >> cpu >> user_ticks >> nice_ticks >> system_ticks >> idle_ticks 
        >> io_wait_ticks >> irq_ticks >> soft_irq_ticks;

    unsigned long long total_ticks = user_ticks + nice_ticks + system_ticks + idle_ticks + io_wait_ticks + irq_ticks + soft_irq_ticks;
    unsigned long long total_ticks_diff = total_ticks - prev_total_ticks;
    unsigned long long idle_ticks_diff = idle_ticks - prev_idle_ticks;

    prev_total_ticks = total_ticks;
    prev_idle_ticks = idle_ticks;

    if (total_ticks_diff == 0) {
        cpu_usage = 0.0;
        return -1;
    }

    cpu_usage = 100.0 - (static_cast<double>(idle_ticks_diff) / total_ticks_diff) * 100.0;


    std::stringstream log_str;
    log_str << "CPU使用率：" << cpu_usage ;
    LOG_DEBUG(log_str.str());

    return 0;

}
int Client::get_mem_info(double& mem_total, double& mem_used, 
                     double& swap_total,double& swap_used)
{
    std::ifstream meminfo_file("/proc/meminfo");
    if( !meminfo_file.is_open() )
    {
        LOG_DEBUG("/proc/meminfo 打开失败！");
        return -1;
    }
    std::string line;
    unsigned long long mem_total_kb = 0, mem_free_kb = 0, mem_buffer_kb = 0,
                       mem_cached_kb = 0, swap_total_kb = 0, swap_free_kb = 0;
    while (std::getline(meminfo_file, line)) {
        std::istringstream iss(line);
        std::string key;
        unsigned long long value;
        iss >> key >> value;
        if (key == "MemTotal:") {
            mem_total_kb = value;
        }
        else if (key == "MemFree:") {
            mem_free_kb = value;
        }
        else if (key == "Buffers:") {
            mem_buffer_kb = value;
        }
        else if (key == "Cached:") {
            mem_cached_kb = value;
        }
        else if (key == "SwapTotal:") {
            swap_total_kb = value;
        }
        else if (key == "SwapFree:") {
            swap_free_kb = value;
        }
    }

    unsigned long long mem_used_kb = mem_total_kb - mem_free_kb - mem_buffer_kb - mem_cached_kb;
    unsigned long long swap_used_kb = swap_total_kb - swap_free_kb;

    mem_total = static_cast<double>(mem_total_kb) / 1024.0;
    mem_used = static_cast<double>(mem_used_kb) / 1024.0;
    swap_total = static_cast<double>(swap_total_kb) / 1024.0;
    swap_used = static_cast<double>(swap_used_kb) / 1024.0;

    std::stringstream log_str;
    log_str << "总内存：" << mem_total << " 已使用：" << mem_used << " 总交换：" << swap_total << "已使用：" << swap_used ;
    LOG_DEBUG(log_str.str());

    return 0;
}
int Client::get_disk_info(double& disk_total, double& disk_used)
{
    struct statvfs vfs;
    if (statvfs("/", &vfs) == 0) {
        disk_total = vfs.f_blocks * vfs.f_frsize / 1024.0 / 1024 / 1024;
        disk_used = (vfs.f_blocks - vfs.f_bfree) * vfs.f_frsize / 1024.0 / 1024 / 1024;
    } else {
        return -1;
    }

    std::stringstream log_str;
    log_str << "总磁盘：" << disk_total << " GB" << " 已使用：" << disk_used << "GB" ;
    LOG_DEBUG(log_str.str());

    return 0;
}
int Client::get_process_number(int& process_number)
{
    DIR* dirp = opendir("/proc");
    if (dirp == nullptr) {
        return -1;  // 打开目录失败
    }

    process_number = 0;
    dirent* dp;
    while ((dp = readdir(dirp)) != nullptr) {
        if (dp->d_type != DT_DIR) {
            continue;
        }
        const char* name = dp->d_name;
        while (*name && std::isdigit(*name)) {
            ++name;
        }
        if (*name != '\0') {
            continue;
        }
        process_number++;
    }
    closedir(dirp);

    return 0;
    
}
int Client::get_os_name(std::string& os_name)
{
    std::ifstream os_release("/etc/os-release");
    if (!os_release.is_open()) {
        LOG_DEBUG("/proc/os-release 打开失败！");
        return -1;
    }
    std::string line;
    while (std::getline(os_release, line)) {
        if (line.find("PRETTY_NAME=") != std::string::npos) {
            std::string distro = line.substr(line.find("=") + 1);
            os_name = distro.substr(1, distro.length() - 2);
            return 0;
        }
    }
    return -2;
}
int Client::get_total_network_usage(double& recv_total_gb,double& sent_total_gb)
{
    // 初始化总发送和接收字节数
    unsigned long long total_sent_bytes = 0;
    unsigned long long total_recv_bytes = 0;

    // 打开/proc/net/dev文件
    std::ifstream file("/proc/net/dev");
    if (!file.is_open()) {
        LOG_DEBUG("/proc/net/dev 打开失败！");
        return -1;
    }

    // 读取文件中的每一行
    std::string line;
    std::getline(file, line);
    std::getline(file, line);

    while (std::getline(file, line)) {
        // 使用字符串流解析每行内容，获取网络接口名、接收字节数、发送字节数
        std::string iface;
        unsigned long long recv_bytes, sent_bytes;
        std::stringstream ss(line);
        std::string dummy;
        ss >> iface >> recv_bytes >> dummy >> dummy >> dummy
           >> dummy >> dummy >> dummy >> dummy >> sent_bytes;
        // 如果网络接口名为lo，则跳过不计入总数
        if (iface == "lo:") {
            continue;
        }

        // 累加接收和发送字节数到总数中
        total_recv_bytes += (recv_bytes/1024/1024);
        total_sent_bytes += (sent_bytes/1024/1024);
    }

    recv_total_gb = total_recv_bytes / 1024.0;
    sent_total_gb = total_sent_bytes / 1024.0;

    std::stringstream log_str;
    log_str << "总接收：" << recv_total_gb << " GB" << " 总发送：" << sent_total_gb << "GB" ;
    LOG_DEBUG(log_str.str());

    return 0;
}
int Client::get_net_speed(double& upload_speed,double& download_speed)
{
    // 打开 /proc/net/dev 文件
    std::ifstream infile("/proc/net/dev");
    if (!infile.is_open()) {
        LOG_DEBUG("/proc/net/dev 打开失败！");
        return -1;
    }

    // 跳过前两行
    std::string line;
    std::getline(infile, line);
    std::getline(infile, line);

    // 计算总的接收和发送字节数
    time_t now = time(nullptr);
    double rx_total = 0.0, tx_total = 0.0;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string iface;
        iss >> iface;
        if (iface.find("lo") != std::string::npos) {
            // 接口名称包含 "lo"，表示本地回环接口，跳过
            continue;
        }
        unsigned long long rx_bytes, tx_bytes;
        std::string dummy;
        iss >> rx_bytes >> dummy >> dummy >> dummy >> dummy
            >> dummy >> dummy >> dummy >> tx_bytes;
        rx_total += rx_bytes;
        tx_total += tx_bytes;
    }

    // 计算时间间隔和速度
    static time_t last_time = now;
    static double last_rx_total = rx_total, last_tx_total = tx_total;
    double interval = difftime(now, last_time);
    if (interval > 0.0) {
        download_speed = (rx_total - last_rx_total) / interval;
        upload_speed = (tx_total - last_tx_total) / interval;
        last_rx_total = rx_total;
        last_tx_total = tx_total;
        last_time = now;

        std::stringstream log_str;
        log_str << "上传速度：" << upload_speed << " KB/s" << " 下载速度：" << download_speed << " KB/s";
        LOG_DEBUG(log_str.str());

        return 0;
    } else {
        // 时间间隔为 0，返回错误
        return -2;
    }
}

void Client::run()
{
    // 创建 UDP 套接字
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if( sockfd < 0)
    {
        std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
        return ;
    }

    // 设置服务器地址
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(client_config_.server_ip.c_str());
    servaddr.sin_port = htons(client_config_.port);

    while(!stop_)
    {
        // 更新客户端数据
        get_client_data();

        // 创建 json 对象
        json client_data_json;
        client_data_json["id"] = client_data_.id;
        client_data_json["os_name"] = client_data_.os_name;
        client_data_json["cpu_number"] = client_data_.cpu_number; 
        client_data_json["cpu_usage"] = client_data_.cpu_usage; 
        client_data_json["mem_total"] = client_data_.mem_total; 
        client_data_json["mem_used"] = client_data_.mem_used; 
        client_data_json["swap_total"] = client_data_.swap_total; 
        client_data_json["swap_used"] = client_data_.swap_used;  
        client_data_json["disk_total"] = client_data_.disk_total; 
        client_data_json["disk_used"] = client_data_.disk_used;
        client_data_json["process_number"] = client_data_.process_number; 
        client_data_json["loadavg_one_min"] = client_data_.loadavg_one_min; 
        client_data_json["loadavg_five_min"] = client_data_.loadavg_five_min;
        client_data_json["loadavg_fifteen_min"] = client_data_.loadavg_fifteen_min;
        client_data_json["recv_total_gb"] = client_data_.recv_total_gb;
        client_data_json["sent_total_gb"] = client_data_.sent_total_gb;
        client_data_json["upload_speed"] = client_data_.upload_speed;
        client_data_json["download_speed"] = client_data_.download_speed;
        std::string json_str = client_data_json.dump();

        std::stringstream log_str;
        log_str << "创建Json成功。";
        LOG_INFO(log_str.str());
        LOG_DEBUG(json_str);

        std::string encrypt_json = srvmon::aes_encrypt(json_str, client_config_.aes_key);
        LOG_INFO("数据已加密。");
        LOG_DEBUG(encrypt_json);

        int ret = sendto(sockfd, encrypt_json.c_str(),encrypt_json.length(),0,
                        (struct sockaddr *)&servaddr, sizeof(servaddr));
        if(ret < 0)
        {
            std::stringstream log_str;
            log_str << "发送失败: " << strerror(errno) ;
            LOG_ERROR(log_str.str());
        }
        else
        {
            std::stringstream log_str;
            log_str << "成功发送 " << ret << " 字节数据" ;
            LOG_INFO(log_str.str());
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }

    // 关闭套接字
    if (close(sockfd) < 0) {
        std::stringstream log_str;
        log_str << "关闭套接字失败: " << strerror(errno) ;
        LOG_ERROR(log_str.str());
    }
}

Client::~Client()
{

}


} // namespace