#ifndef UTILS_H
#define UTILS_H

#include <functional>
#include <cstddef>
#include <iostream>
#include <cstdlib>
#include <curl/curl.h>
#include <string>
#include <openssl/aes.h>
#include "nlohmann/json.hpp"

namespace srvmon{

size_t write_callback(char* ptr, size_t size, size_t nmemb, std::string* data);

bool send_to_wecom(const std::string& text,
                    const std::string& wecom_cid,
                    const std::string& wecom_aid,
                    const std::string& wecom_secret,
                    const std::string& wecom_touid = "@all");

// AES加密函数
std::string aes_encrypt(std::string input, std::string aes_key);

// AES解密函数
std::string aes_decrypt(std::string input, std::string aes_key);


}

#endif