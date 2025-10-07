#include "client_list.h"
#include "server_agreement.h"
// 初始化客户端链表
ClientList *client_list_init()
{
    ClientList *list = (ClientList *)malloc(sizeof(ClientList));
    if (list == NULL)
    {
        perror("client_list_init: malloc failed");
        return NULL;
    }

    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
    pthread_mutex_init(&list->mutex, NULL); // 初始化互斥锁

    return list;
}

// 销毁客户端链表
void client_list_destroy(ClientList *list)
{
    if (list == NULL)
        return;

    pthread_mutex_lock(&list->mutex); // 加锁

    ClientNode *current = list->head;
    ClientNode *next = NULL;

    // 遍历链表，释放所有节点
    while (current != NULL)
    {
        next = current->next;
        free(current);
        current = next;
    }

    list->head = NULL;
    list->tail = NULL;
    list->count = 0;

    pthread_mutex_unlock(&list->mutex);  // 解锁
    pthread_mutex_destroy(&list->mutex); // 销毁互斥锁
    free(list);
}

// 插入新客户端（线程安全）
ClientNode *client_list_insert(ClientList *list, const char *ip, const int *sock_fd)
{
    if (list == NULL)
    {
        fprintf(stderr, "client_list_insert: list is NULL\n");
        return NULL;
    }

    // 创建新节点
    ClientNode *new_node = (ClientNode *)malloc(sizeof(ClientNode));
    memset(new_node, 0, sizeof(ClientNode));
    if (new_node == NULL)
    {
        perror("client_list_insert: malloc failed");
        return NULL;
    }

    strncpy(new_node->ip, ip, 16);
    new_node->connect_time = time(NULL); // 记录当前时间戳
    new_node->sock_fd = *sock_fd;
    new_node->send_count = 0;
    new_node->prev = NULL;
    new_node->next = NULL;

    pthread_mutex_lock(&list->mutex); // 加锁

    // 链表为空
    if (list->head == NULL)
    {
        list->head = new_node;
        list->tail = new_node;
    }
    else
    {
        // 插入到链表头部（也可根据需求插入到尾部或其他位置）
        new_node->next = list->head;
        list->head->prev = new_node;
        list->head = new_node;
    }

    list->count++;                      // 增加在线客户端数量
    pthread_mutex_unlock(&list->mutex); // 解锁
    return new_node;
}

// 登录后插入客户端（线程安全）
int client_login_insert(ClientList *list, const char *username, const int *sock_fd)
{
    if (list == NULL)
    {
        fprintf(stderr, "client_list_insert: list is NULL\n");
        return -1;
    }
    pthread_mutex_lock(&list->mutex); // 加锁

    ClientNode *current = list->head;
    int count = 1;
    // 遍历链表，调用回调函数
    while (current != NULL)
    {
        if (current->sock_fd == *sock_fd && strcmp(current->username, "") == 0)
        {
            strcpy(current->username, username);
            break;
        }
        // count++;
        current = current->next;
    }

    pthread_mutex_unlock(&list->mutex); // 解锁
    return 0;
}
// 删除客户端（根据sock_fd，线程安全）
int client_list_delete(ClientList *list, const int *sock_fd)
{
    if (list == NULL)
    {
        fprintf(stderr, "client_list_delete: list is NULL\n");
        return -1;
    }

    pthread_mutex_lock(&list->mutex); // 加锁

    ClientNode *current = list->head;

    // 遍历链表查找目标节点
    while (current != NULL)
    {
        if (current->sock_fd == *sock_fd)
        {
            // 处理前驱节点
            if (current->prev != NULL)
            {
                current->prev->next = current->next;
            }
            else
            {
                list->head = current->next; // 删除头节点
            }

            // 处理后继节点
            if (current->next != NULL)
            {
                current->next->prev = current->prev;
            }
            else
            {
                list->tail = current->prev; // 删除尾节点
            }

            // 释放节点内存
            free(current);
            list->count--; // 减少在线客户端数量

            pthread_mutex_unlock(&list->mutex); // 解锁
            return 0;
        }
        current = current->next;
    }

    pthread_mutex_unlock(&list->mutex); // 解锁
    return -1;                          // 未找到目标节点
}
// 用户退出（根据username，线程安全）
int client_logout_delete(ClientList *list, const int *sock_fd)
{

    if (list == NULL)
    {
        fprintf(stderr, "client_list_insert: list is NULL\n");
        return 0;
    }
    pthread_mutex_lock(&list->mutex); // 加锁

    ClientNode *current = list->head;
    int count = 1;
    // 遍历链表
    while (current != NULL)
    {
        if (current->sock_fd == *sock_fd)
        {
            strcpy(current->username, "");
            current->send_count = 0;
            break;
        }
        // count++;
        current = current->next;
    }

    pthread_mutex_unlock(&list->mutex); // 解锁
    return 1;
}

// 遍历客户端链表,返回在线用户列表
void client_logined_traverse(ClientList *list, Packet *relypack)
{
    if (list == NULL)
    {
        return;
    }

    pthread_mutex_lock(&list->mutex); // 加锁

    ClientNode *current = list->head;
    // int count=1;
    if (current != NULL) // 整理发出的信息格式
    {
        if (strcmp(current->username, "") != 0)
        {
            strcat(relypack->data, current->username);
        }

        current = current->next;
    }
    while (current != NULL && strcmp(current->username, "") != 0)
    {

        if (strcmp(current->username, "") != 0)
        {
            strcat(relypack->data, ",");
            strcat(relypack->data, current->username);
        }
        // count++;
        current = current->next;
    }

    pthread_mutex_unlock(&list->mutex); // 解锁
}

// 查询客户端（根据用户名，线程安全）
ClientNode *client_list_username_find(ClientList *list, const char *username)
{
    if (list == NULL)
    {
        fprintf(stderr, "client_list_username_find: list is NULL\n");
        return NULL;
    }

    pthread_mutex_lock(&list->mutex); // 加锁

    ClientNode *current = list->head;

    // 遍历链表查找目标节点
    while (current != NULL)
    {
        if (strcmp(current->username, username) == 0)
        {
            pthread_mutex_unlock(&list->mutex); // 解锁
            return current;
        }
        current = current->next;
    }

    pthread_mutex_unlock(&list->mutex); // 解锁
    return NULL;                        // 未找到目标节点
}

// 查询客户端（根据sock_fd，线程安全）
ClientNode *client_list_socketid_find(ClientList *list, const int *sock_fd)
{
    if (list == NULL)
    {
        fprintf(stderr, "client_list_socketid_find: list is NULL\n");
        return NULL;
    }

    pthread_mutex_lock(&list->mutex); // 加锁

    ClientNode *current = list->head;

    // 遍历链表查找目标节点
    while (current != NULL)
    {
        if (current->sock_fd == *sock_fd)
        {
            pthread_mutex_unlock(&list->mutex); // 解锁
            return current;
        }
        current = current->next;
    }

    pthread_mutex_unlock(&list->mutex); // 解锁
    return NULL;                        // 未找到目标节点
}
// 获取客户端的当前账户名
char *client_get_username(ClientList *list, const int *sock_fd)
{
    if (list == NULL)
    {
        fprintf(stderr, "client_get_username: list is NULL\n");
        return NULL;
    }
    pthread_mutex_lock(&list->mutex); // 加锁

    ClientNode *current = list->head;

    // 遍历链表查找目标节点
    while (current != NULL)
    {
        if (current->sock_fd == *sock_fd)
        {
            pthread_mutex_unlock(&list->mutex); // 解锁
            return current->username;
        }
        current = current->next;
    }

    pthread_mutex_unlock(&list->mutex); // 解锁
    return NULL;                        // 未找到目标节点
}
