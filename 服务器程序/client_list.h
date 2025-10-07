#ifndef CLIENT_LIST_H
#define CLIENT_LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>    // 用于记录连接时间
#include <pthread.h> // 用于线程安全（互斥锁）

#include "server_agreement.h"
// // 前向声明 Packet 类型，替代 #include "server_agreement.h"
// typedef struct Packet Packet;
// 客户端会话节点结构
typedef struct ClientNode
{
    char ip[16];             // 客户端IP地址（IPv4）
    time_t connect_time;     // 连接时间（时间戳）
    char username[64];       // 用户名
    int sock_fd;             // Accept的客户端套接字描述符
    int send_count;          // 发送消息数
    struct ClientNode *prev; // 指向前一个节点
    struct ClientNode *next; // 指向后一个节点
} ClientNode;

// 客户端链表结构（头节点 + 尾节点 + 互斥锁）
typedef struct ClientList
{
    ClientNode *head;      // 链表头节点
    ClientNode *tail;      // 链表尾节点
    pthread_mutex_t mutex; // 互斥锁（线程安全）
    int count;             // 在线客户端数量
} ClientList;

// 初始化客户端链表
extern ClientList *client_list;
ClientList *client_list_init();

// 销毁客户端链表
void client_list_destroy(ClientList *list);

// 插入新客户端（线程安全）
ClientNode *client_list_insert(ClientList *list, const char *ip, const int *sock_fd);

// 登录后插入客户端（线程安全）
int client_login_insert(ClientList *list, const char *username, const int *sock_fd);

// 删除客户端（根据sock_fd，线程安全）
int client_list_delete(ClientList *list, const int *sock_fd);

// 用户退出（根据username，线程安全）
int client_logout_delete(ClientList *list, const int *sock_fd);

// 查询客户端（根据username，线程安全）
ClientNode *client_list_username_find(ClientList *list, const char *username);
// 查询客户端（根据sock_fd，线程安全）
ClientNode *client_list_socketid_find(ClientList *list, const int *sock_fd);
// 获取客户端的当前账户名
char *client_get_username(ClientList *list, const int *sock_fd);

// 遍历客户端链表,返回在线用户列表
void client_logined_traverse(ClientList *list, Packet *relypack);

#endif