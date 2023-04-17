#include <iostream>
#include "nlohmann/json.hpp"
#include "log.h"
#include "utils.h"
#include "server.h"
#include "client.h"

using json = nlohmann::json;

int main()
{
    int select = 0;

    std::cout << "请选择启动客户端还是服务器：" << std::endl
              << "1. 服务器" << std::endl
              << "2. 客户端" << std::endl;
    
    std::cin >> select;
    while(select != 1 && select != 2)
    {
        std::cout << "输入有误，请重新输入：";
        std::cin >> select;
    }

    if(select == 1)
    {
        srvmon::Server_monitor server_monitor("./server.config.json");
        server_monitor.run();
    }
    else if(select == 2)
    {
        srvmon::Client client("./client.config.json");
        client.run();
    }
    
    return 0;
}