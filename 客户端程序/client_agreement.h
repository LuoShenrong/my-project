#ifndef __CLIENT_AGREEMENT_H__
#define __CLIENT_AGREEMENT_H__
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
// 网络编程核心头文件，包含 socket、bind 等函数定义，以及 AF_INET、SOCK_STREAM 等常量
#include <sys/types.h>
#define BUFFER_SIZE 1024
typedef struct
{
    int32_t data_type;      // 报文类型
    int32_t size;           // 有效字节数
    char data[BUFFER_SIZE]; // 数据
} Packet;
enum
{
    TYPE_LOGIN, // 登陆数据包
    TYPE_REG,   // 注册数据包
    TYPE_CHANGE_PASSWORD,
    TYPE_MSG,    // 消息数据包
    TYPE_HEART,  // 心跳数据包
    TYPE_CMD,    //  命令数据包
    TYPE_ONLINE, // 查看在线用户
    TYPE_LOGOUT, // 退出登录
    TYPE_OK,
    TYPE_ERR
};
Packet create_reg_packet(const char *username, const char *password);
Packet create_login_packet(const char *username, const char *password);
Packet create_change_password_packet(const char *username, const char *oldpassword, const char *newpassword);
Packet create_msg_packet(const char *username, const char *msg);
Packet create_cmd_packet(const char *cmd);

#endif
