#include <stdio.h>
// #include<server.h>
#include "thread.h"
#include <signal.h>

ClientList *client_list = NULL;
pthread_rwlock_t accout_rwlock;
pthread_mutex_t datalog_mutex;
void sigint_handler(int signum)
{
    printf("收到 SIGINT 信号，程序准备优雅退出...\n");
    // 这里可以添加资源清理代码，比如关闭文件、释放内存等
    client_list_destroy(client_list);      // 内部已删除锁
    pthread_mutex_destroy(&datalog_mutex); // 删除锁
    printf("资源清理完毕，程序退出\n");
    exit(0); // 也可以在这里主动退出程序
}

bool read_server_config(char *ip, int *port)
{

    FILE *file = fopen("ipconfig.txt", "r");
    if (file == NULL)
    {
        perror("Failed to open config file");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    while (fgets(buffer, BUFFER_SIZE, file) != NULL)
    {
        // 去除换行符
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n')
        {
            buffer[len - 1] = '\0';
        }

        // 解析服务器IP
        if (strncmp(buffer, "SERVER_IP = ", 12) == 0)
        {
            strcpy(ip, buffer + 12);
        }
        // 解析端口
        else if (strncmp(buffer, "PORT = ", 7) == 0)
        {
            *port = atoi(buffer + 7);
        }
    }

    fclose(file);
    return true;
}
// 创建客户端线程管理线程
void *client_thread_manage(void *arg)
{
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0)
    {
        perror("socket");
        return NULL; // Error creating socket
    }
    char ip[16];
    int port;
    if (!read_server_config(ip, &port))
    {
        printf("read_server_config failed\n");
        return NULL;
    }
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip); // 自动监听所有可用连接
    addr.sin_port = htons(port);
    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        close(s);
        return NULL; // Error binding socket
    }

    if (listen(s, 5) < 0)
    {
        perror("listen");
        close(s);
        return NULL; // Error listening on socket
    }
    // printf("TCP server ip=%s,port=%d\n",ip,port);
    pthread_t thread_id[5];
    memset(&thread_id, 0, sizeof(thread_id));
    int i = 0;
    client_list = client_list_init();
    if (client_list == NULL)
    {
        printf("client_list_init failed\n");
        return NULL;
    }
    // 互斥访问资源
    pthread_rwlock_init(&accout_rwlock, NULL);
    pthread_mutex_init(&datalog_mutex, NULL); // 初始化
    while (1)
    {

        ThreadArgs *client_info = (ThreadArgs *)malloc(sizeof(ThreadArgs));
        memset(client_info, 0, sizeof(ThreadArgs));
        struct sockaddr_in client_addr;
        socklen_t addrlen = sizeof(client_addr);
        client_info->client_socket = accept(s, (struct sockaddr *)&client_addr, &addrlen);

        strcpy(client_info->ip, inet_ntoa(client_addr.sin_addr));
        // printf("TCP echo server have something messages\n");
        if (client_info->client_socket < 0)
        {
            perror("accept");
            free(client_info); // 释放堆内存
            continue;          // 跳过创建线程
        }
        pthread_create(&thread_id[i], NULL, &handle_client, (void *)client_info);
        i++;
    }
}

void client_list_show(ClientList *list)
{
    if (list == NULL)
    {
        printf("client_list is NULL\n");
        return;
    }
    printf("用户        ip地址           sockid        发送消息数         上线时间 \n");
    ClientNode *node = list->head;
    char node_time[32];
    time_t node_time_t;

    while (node != NULL)
    {
        if (strcmp(node->username, "") != 0)
        {
            node_time_t = node->connect_time;
            get_standard_time(node_time, &node_time_t);
            printf("%-10s %-20s %-10d %-15d %-15s\n", node->username, node->ip, node->sock_fd, node->send_count, node_time);
        }

        node = node->next;
    }
}
int main()
{
    // 注册 SIGINT 信号处理函数
    signal(SIGINT, sigint_handler);
    pthread_t thread_manage;

    int ret = pthread_create(&thread_manage, NULL, client_thread_manage, NULL);
    // pthread_detach(thread_manage);
    if (ret != 0)
    {
        printf("创建子线程失败\n");
        return 1;
    }
    printf("当前服务端可选指令如下：\n");
    printf("1.show sesssion\n");
    printf("2.退出服务端\n");
    char choice[10];
    while (1)
    {
        printf("请选择指令编号：");
        memset(&choice, 0, sizeof(choice));
        scanf("%s", choice);
        if (strcmp(choice, "1") == 0)
        {
            client_list_show(client_list);
        }
        else if (strcmp(choice, "2") == 0)
        {
            client_list_destroy(client_list);      // 内部已删除锁
            pthread_mutex_destroy(&datalog_mutex); // 删除锁
            break;
        }
        else
        {
            printf("输入错误\n");
        }
    }
}