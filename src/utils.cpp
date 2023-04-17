
#include "utils.h"
#include "log.h"

namespace srvmon{

using json = nlohmann::json;

// AES加密函数
std::string aes_encrypt(std::string input, std::string aes_key)
{
    if(aes_key.size() != 16)
    {
        std::cerr << "当前 aes_key 长度为" << aes_key.size() << ", 请将其长度设置为16。" << std::endl;
        exit(-16);
    }
    char aes_key_buf[aes_key.size() + 1];
    std::copy(aes_key.begin(), aes_key.end(), aes_key_buf);
    aes_key_buf[aes_key.size()] = '\0';

    int input_len = input.length();
    int padding_len = AES_BLOCK_SIZE - input_len % AES_BLOCK_SIZE;
    int output_len = input_len + padding_len;

    unsigned char *input_buf = new unsigned char[output_len];
    memcpy(input_buf, input.c_str(), input_len);
    memset(input_buf + input_len, (unsigned char)padding_len, padding_len);

    unsigned char iv[AES_BLOCK_SIZE];
    memset(iv, 0x00, AES_BLOCK_SIZE);

    AES_KEY aes_key_struct;
    AES_set_encrypt_key(reinterpret_cast<const unsigned char *>(aes_key_buf), aes_key.size() * 8, &aes_key_struct);

    unsigned char *output_buf = new unsigned char[output_len];
    AES_cbc_encrypt(input_buf, output_buf, output_len, &aes_key_struct, iv, AES_ENCRYPT);

    std::string output(reinterpret_cast<char const *>(output_buf), output_len);

    delete[] input_buf;
    delete[] output_buf;

    return output;
}

// AES解密函数
std::string aes_decrypt(std::string input, std::string aes_key)
{
    if(aes_key.size() != 16)
    {
        std::cerr << "当前 aes_key 长度为" << aes_key.size() << ", 请将其长度设置为16。" << std::endl;
        exit(-16);
    }
    char aes_key_buf[aes_key.size() + 1];
    std::copy(aes_key.begin(), aes_key.end(), aes_key_buf);
    aes_key_buf[aes_key.size()] = '\0';

    int input_len = input.length();

    unsigned char iv[AES_BLOCK_SIZE];
    memset(iv, 0x00, AES_BLOCK_SIZE);

    AES_KEY aes_key_struct;
    AES_set_decrypt_key(reinterpret_cast<const unsigned char *>(aes_key_buf), aes_key.size() * 8, &aes_key_struct);

    unsigned char *output_buf = new unsigned char[input_len];
    AES_cbc_encrypt(reinterpret_cast<const unsigned char *>(input.c_str()), output_buf, input_len, &aes_key_struct, iv, AES_DECRYPT);

    int padding_len = output_buf[input_len - 1];
    int output_len = input_len - padding_len;

    std::string output(reinterpret_cast<char const *>(output_buf), output_len);

    delete[] output_buf;

    return output;
}

// size_t write_callback(char* ptr, size_t size, size_t nmemb, std::string* data)
// {
//     data->append(ptr, size * nmemb);
//     return size*nmemb;
// }

} // namespace