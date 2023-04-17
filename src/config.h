#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <iostream>
#include <fstream>
#include "nlohmann/json.hpp"
#include "log.h"
#include "definitions.h"


namespace srvmon{

using json = nlohmann::json;

class Config : public srvmon::disable_copying_and_assignment
{

public:
    Config(const std::string& config_file_name);
    virtual ~Config();
    int load_config(srvmon::Server_config& server_config) const;
    int load_config(srvmon::Client_config& client_config) const;

private:
    std::string config_file_name_;
};


}


#endif