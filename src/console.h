#ifndef CONSOLE_H
#define CONSOLE_H

#include <iostream>
#include <iomanip>
#include <string>
#include <unordered_map>
#include <chrono>
#include <memory>
#include <algorithm>
#include <cstdlib>

#include "definitions.h"
#include "log.h"
#include "notice.h"

namespace srvmon{

enum class ConsoleColor {
    Black = 30,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta,
    Cyan,
    White
};

enum class Sort_status{
    Default = 0,
    CPU_ASC,
    CPU_DESC,
    MEM_ASC,
    MEM_DESC
};

class Console : public disable_copying_and_assignment
{

public:
    Console(const std::vector<srvmon::Client_data>& data_center,std::shared_ptr<srvmon::Notice>);
    virtual ~Console();
    void print(const std::vector<srvmon::Client_data>& data_center);
    void check_input();

private:
    void progress_bar(double progress,int total = 100, int bar_width = 50);
    void print_client_data(const srvmon::Client_data&);
    void print_help();
    void print_title();
    void clear_screen();
    bool online_check(const srvmon::Client_data&);
    


private:
    std::vector<srvmon::Client_data> vec_client_data_;
    std::unordered_map<std::string, bool> is_online_;
    std::shared_ptr<srvmon::Notice> notice_;
    const std::string color_start_;
    const std::string color_end_;
    srvmon::Sort_status cur_sort_status_;

};

} // namespace



#endif