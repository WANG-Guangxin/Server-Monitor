## Server Monitor CPP Demo Project

### Feature

### 库依赖

1. OpenSSL
> sudo apt install build-essential
> 
> sudo apt install openssl libssl-dev

2. nlohmann/json.hpp

3. curl
> sudo apt install curl

### 目录结构

| /----- |                      |      |      |      |
| :----: | -------------------- | ---- | ---- | ---- |
|   \|   | —include/           | ：   |  库文件    |      |
|   \|   | —log/               | ：   |    日志文件  |      |
|   \|   | —obj/               | ：   |   目标文件   |      |
|   \|   | —src/               | ：   |   项目源文件   |      |
|   \|   | —Makefile           | ：   |      |      |
|   \|   | —bulid.sh           | ：   |  构建脚本    |      |
|   \|   | —client.config.json | ：   |   客户端配置文件   |      |
|   \|   | —server.config.json | ：   |   服务端配置文件   |      |



### 配置文件说明

服务端配置文件

```cpp
{
    // 服务端绑定IP
    "listen_ip": "0.0.0.0",
    // 服务端端口
    "port": 6789,
    // AES加密密钥，注意应该是长度为 16 的字符串，且与客户端保持一直
    "aes_key": "1234567890abcdef",
    // 日志级别，0：Debug，1：Info，2：Warning，3：Error，>=4 : 关闭日志
    "log_level": 0,
    // 监控服务器数组
    "servers":[
        {
            // 唯一标识，与客户端保持一直
            "id": "111",
            // 名字，仅用于显示
            "name": "第一个",
            // 分组名，暂时没用
            "group_name": "第一组"
        },
        {
            "id": "222",
            "name": "第二个",
            "group_name": "第一组"
        },
        {
            "id": "333",
            "name": "第三个",
            "group_name": "第二组"
        }
    ],
    // 报警规则
    "alarm_rules":{
        // 离线、上线报警
        "offline": true,
        // CPU占用率报警
        "cpu": true,
        // CPU 占用率阈值，范围0~1，超过此数值触发报警
        "cpu_threshold": 0.9,
        // 内存占用率报警
        "memory": true,
        // 内存占用率报警阈值
        "memory_threshold": 0
    },
    // 通知方式
    "notice":[
        {
            // 是否开启 Telegram 推送
            "notice_tg": true,
            // Telegram 机器人 Token
            "tg_token": "",
            // 对话ID
            "chat_id": ""
        },
        {
            // 是否开启企业微信机器人推送
            "notice_wechat": true,
            // 企业ID
            "qiye_id": "",
            // 用户ID
            "agent_id": "",
            // 密钥
            "secret": ""
        }
    ]
}
```

客户端配置文件

```cpp
{
    // 客户端ID
    "id": "111",
    // 服务端IP地址
    "server_ip": "127.0.0.1",
    // 服务端端口
    "port": 6789,
    // 加密密钥
    "aes_key": "1234567890abcdef",
    // 日志级别
    "log_level": 0
}
```




### 启动说明

### 开发说明

TODO:

> 1. web-ui


### 更新日志

2023-04-21  V0.9
> init.