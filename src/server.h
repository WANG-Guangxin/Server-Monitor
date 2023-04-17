#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <vector>
#include <memory>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sstream>
#include <unistd.h>
#include <chrono>
#include <ctime>
#include <thread>
#include "utils.h"
#include "log.h"
#include "definitions.h"
#include "config.h"
#include "console.h"
#include "notice.h"

namespace srvmon{

using json = nlohmann::json;

class Server_monitor : public disable_copying_and_assignment
{
public:
    Server_monitor(const std::string& config_file_name = "server.config.json");
    virtual ~Server_monitor();
    void run();
    int accept_data();
private:
    bool stop_;
    std::unique_ptr<srvmon::Config> config_;
    srvmon::Server_config server_config_;
    std::vector<srvmon::Client_data> data_center_;
    std::unordered_map<std::string, size_t> id_to_idx_;
    std::unique_ptr<srvmon::Console> console_;
    std::shared_ptr<srvmon::Notice> notice_;
};



}  // namespace

#endif