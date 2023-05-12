#include "console.h"

namespace srvmon{

Console::Console(const std::vector<srvmon::Client_data>& data_center,
                 std::shared_ptr<srvmon::Notice> notice)
    :
    color_start_("\033["),
    color_end_("\033[0m"),
    cur_sort_status_(srvmon::Sort_status::Default),
    vec_client_data_(data_center),
    notice_(notice)
{
    for(auto& client_data:vec_client_data_)
    {
        is_online_.insert({client_data.id,online_check(client_data)});
    }
}

void Console::print(const std::vector<srvmon::Client_data>& data_center)
{
    vec_client_data_ = data_center;
    switch(cur_sort_status_)
    {
        case srvmon::Sort_status::CPU_ASC:
        {
            std::sort(vec_client_data_.begin(),vec_client_data_.end(),[](
                const srvmon::Client_data& data1,const srvmon::Client_data& data2
            ){
                return data1.cpu_usage < data2.cpu_usage;
            });
            break;
        }
        case srvmon::Sort_status::CPU_DESC:
        {
            std::sort(vec_client_data_.begin(),vec_client_data_.end(),[](
                const srvmon::Client_data& data1,const srvmon::Client_data& data2
            ){
                return data1.cpu_usage > data2.cpu_usage;
            });
            break;
        }
        case srvmon::Sort_status::MEM_ASC:
        {
            std::sort(vec_client_data_.begin(),vec_client_data_.end(),[](
                const srvmon::Client_data& data1,const srvmon::Client_data& data2
            ){
                return data1.mem_used/data1.mem_total < data2.mem_used/data2.mem_total;
            });
            break;
        }
        case srvmon::Sort_status::MEM_DESC:
        {
            std::sort(vec_client_data_.begin(),vec_client_data_.end(),[](
                const srvmon::Client_data& data1,const srvmon::Client_data& data2
            ){
                return data1.mem_used/data1.mem_total > data2.mem_used/data2.mem_total;
            });
            break;
        }
        default:
        {

        }
    }

    for(auto& client_data:vec_client_data_)
    {
        is_online_[client_data.id] = online_check(client_data);
    }

    clear_screen();
    print_help();
    print_title();
    for(auto& client_data:vec_client_data_)
    {
        print_client_data(client_data);
    }

    // notice check
    notice_->notice_check(vec_client_data_,is_online_);

}

void Console::check_input()
{
    while(true)
    {
        char c;
        std::cin >> c;
        if( c == 'd')
        {
            if(cur_sort_status_ == srvmon::Sort_status::Default)
            {

            }
            else
            {
                cur_sort_status_ = srvmon::Sort_status::Default;
                print(vec_client_data_);
            }
        }
        else if( c == 'm')
        {
            if(cur_sort_status_ == srvmon::Sort_status::MEM_ASC)
            {
                cur_sort_status_ = srvmon::Sort_status::MEM_DESC;
            }
            else
            {
                cur_sort_status_ = srvmon::Sort_status::MEM_ASC;
            }
            print(vec_client_data_);
        }
        else if( c == 'c')
        {
            if(cur_sort_status_ == srvmon::Sort_status::CPU_ASC)
            {
                cur_sort_status_ = srvmon::Sort_status::CPU_DESC;
            }
            else
            {
                cur_sort_status_ = srvmon::Sort_status::CPU_ASC;
            }
            print(vec_client_data_);
        }
        else
        {

        }
    }
    
}

void Console::progress_bar(double progress,int total, int bar_width)
{
    double fraction = (progress) / total;
    int filledWidth = static_cast<int>(fraction * bar_width);
    int color = static_cast<int>(ConsoleColor::White);

    if (fraction < 0.5f) {
        color = static_cast<int>(ConsoleColor::Green);
    } else if (fraction >= 0.5f && fraction < 0.9f) {
        color = static_cast<int>(ConsoleColor::Yellow);
    } else {
        color = static_cast<int>(ConsoleColor::Red);
    }

    std::cout << "\033[" << color << "m[";
    for (int i = 0; i < filledWidth; ++i) {
        std::cout << "=";
    }

    if (filledWidth < bar_width) {
        std::cout << ">";
    }

    for (int i = 0; i < bar_width - filledWidth - 1; ++i) {
        std::cout << " ";
    }
    std::cout << "] " << static_cast<int>(fraction * 100) << "%\033[0m";
}

void Console::print_client_data(const srvmon::Client_data& client_data)
{
    std::cout << "Server ID: " << client_data.id  << "\tServer name: " << client_data.server_name << "\t";
    if(is_online_[client_data.id] == false)
    {
        std::cout << color_start_ << static_cast<int>(ConsoleColor::Red) << "m"
                  << "离 线" << color_end_ << std::endl;
    }
    else
    {
        std::cout << color_start_ << static_cast<int>(ConsoleColor::Green) << "m"
                  << "在 线" << color_end_ << std::endl;
    }
    std::cout << "Server IP: " << client_data.ip  << "\tServer OS: " << client_data.os_name << std::endl;
    std::cout << "Core Number: " << client_data.cpu_number << "\tloadavg: " << client_data.loadavg_one_min << ", "
              << client_data.loadavg_five_min << ", " << client_data.loadavg_fifteen_min << "\tProcess Number:"
              << client_data.process_number << std::endl; 

    std::cout << " CPU:  " ;
    progress_bar(client_data.cpu_usage);
    std::cout << std::endl;

    std::cout << " Mem:  " ;
    progress_bar(client_data.mem_used / client_data.mem_total * 100);
    std::cout << std::fixed << std::setprecision(2) << " " << client_data.mem_used << "/"
              << client_data.mem_total << "MB  ";
    std::cout << std::endl;

    std::cout << " Disk: ";
    progress_bar(client_data.disk_used / client_data.disk_total * 100); 
    std::cout << std::fixed << std::setprecision(2) << " " << client_data.disk_used << "/"
              << client_data.disk_total << "GB  ";
    std::cout << std::endl;

    std::cout << " Swap: " << std::fixed << std::setprecision(2) << client_data.swap_used/1024.0 << "/"
              << client_data.swap_total/1024.0 << "GB  " << std::endl; 
    std::cout << " Network: " << "↑ " << std::fixed << std::setprecision(2) << client_data.upload_speed << "KB/s | "
              << "↓ " << client_data.download_speed << "KB/s\t" << "↑ " << client_data.sent_total_gb << "GB | " 
              << "↓ " << client_data.recv_total_gb << "GB" << std::endl;
    std::string line(80,'-');
    std::cout << line << std::endl;
}
void Console::print_help()
{
    std::cout << "help: " << std::endl;
    std::cout << "\"c\": sort by cpu ; \"m\": sort by memory ; \"d\": default" << std::endl;
}
void Console::print_title()
{
    std::string padding(33,' ');
    std::cout << color_start_ << static_cast<int>(ConsoleColor::White)+10 << "m" 
              << padding << color_end_
              << color_start_ << "1;" << static_cast<int>(ConsoleColor::White)+10 
              << ";" << static_cast<int>(ConsoleColor::Red) << "m" 
              << "Server" << color_end_
              << color_start_ << static_cast<int>(ConsoleColor::White)+10 << "m" 
              << " " << color_end_
              << color_start_ << "1;" << static_cast<int>(ConsoleColor::White)+10 
              << ";" << static_cast<int>(ConsoleColor::Green) << "m" 
              << "Monitor" << color_end_
              << color_start_ << static_cast<int>(ConsoleColor::White)+10 << "m" 
              << padding << color_end_ << std::endl;
}

bool Console::online_check(const srvmon::Client_data& client_data)
{
    auto now = std::chrono::system_clock::now();
    if( std::chrono::system_clock::to_time_t(now) - client_data.accept_time > 60)
    {
        return false;
    }
    else
    {
        return true;
    }
}

void Console::clear_screen()
{
    std::system("clear");
}

Console::~Console()
{

}


} // namespace
