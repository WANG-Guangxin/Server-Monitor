#ifndef CLIENT_H
#define CLIENT_H

#include <memory>
#include <string>
#include <sstream>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sys/statvfs.h>
#include <dirent.h>
#include <ctime>
#include <chrono>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "definitions.h"
#include "log.h"
#include "config.h"
#include "utils.h"
#include "nlohmann/json.hpp"

namespace srvmon{

using json = nlohmann::json;

class Client{

public:
    Client(const std::string& config_file_name = "client.config.json");
    virtual ~Client();
    void run();

private:
    void get_client_data();

    int get_load_avg(double& one_min, double& five_min, double& fifteen_min);
    int get_cpu_number(int& cpu_number);
    int get_cpu_usage(double& cpu_usage);
    int get_mem_info(double& mem_total, double& mem_used, 
                     double& swap_total,double& swap_used);
    int get_disk_info(double& disk_total, double& disk_used);
    int get_process_number(int& process_number);
    int get_os_name(std::string& os_name);
    int get_total_network_usage(double& recv_total_gb,double& sent_total_gb);
    int get_net_speed(double& upload_speed,double& download_speed);


private:
    bool stop_;
    std::unique_ptr<srvmon::Config> ptr_config_;
    srvmon::Client_config client_config_;
    srvmon::Client_data client_data_;
};

}  // namespace

#endif