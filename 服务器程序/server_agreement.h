#ifndef SERVER_AGREEMENT_H
#define SERVER_AGREEMENT_H
#include <stdio.h>
// 网络编程核心头文件，包含 socket、bind 等函数定义，以及 AF_INET、SOCK_STREAM 等常量
#include <sys/socket.h>
// 包含 sockaddr_in 结构体、INADDR_ANY、htons 等网络相关定义
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <string.h>
// 包含 close 函数（关闭文件描述符，socket 也是一种文件描述符）
#include <unistd.h>
// 可选：如果要用一些系统相关的辅助函数或类型，可加（比如某些环境下可能需要）
#include <sys/types.h>

// 通过前向声明来打破循环依赖：
typedef struct ClientNode ClientNode;
extern pthread_mutex_t datalog_mutex;
#define BUFFER_SIZE 1024
typedef struct
{
    int32_t data_type;      // 报文类型
    int32_t size;           // 有效字节数
    char data[BUFFER_SIZE]; // 数据
} Packet;
typedef struct
{
    int client_socket;
    char ip[16];
} ThreadArgs;
enum
{
    TYPE_LOGIN, // 登陆数据包
    TYPE_REG,   // 注册数据包
    TYPE_CHANGE_PASSWORD,
    TYPE_MSG,    // 消息数据包
    TYPE_HEART,  // 心跳数据包
    TYPE_CMD,    //  命令数据包
    TYPE_ONLINE, // 查看在线用户
    TYPE_LOGOUT,
    TYPE_OK,
    TYPE_ERR
};

// 创建与枚举对应的字符串数组

const char *get_enum_name(int enum_value);
Packet reg_packet(const Packet *packet);
Packet change_password_packet(const Packet *packet);
Packet login_packet(const Packet *packet, const int *client_socket, const char *ip);
Packet cmd_packet(const Packet *packet);
Packet online_packet(const Packet *packet, const int *client_socket, const char *ip);
Packet packet_handle(const Packet *packet, const int *client_socket, const char *ip);
Packet send_messsage(const Packet *packet, const char *send_username);
Packet logout_packet(const int *client_socket);
int get_standard_time(char *receive_time, time_t *receive_time_t);
bool datalog_save(ClientNode *client, const Packet *receive_packet);
bool datalog_save_connect_time(ClientNode *client, char *connect_state);
#endif
