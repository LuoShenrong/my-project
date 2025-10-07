#include "server_agreement.h"
#include "accout.h"
#include "client_list.h"
extern ClientList *client_list;
extern int heart_count;
const char *enum_type_names[] = {
    "TYPE_LOGIN",
    "TYPE_REG",
    "TYPE_CHANGE_PASSWORD",
    "TYPE_MSG",
    "TYPE_HEART",
    "TYPE_CMD",
    "TYPE_ONLINE",
    "TYPE_LOGOUT",
    "TYPE_OK",
    "TYPE_ERR"};

int get_standard_time(char *receive_time, time_t *receive_time_t)
{
    struct tm time_buf; // 用于存储当前时间的缓冲区
    localtime_r(receive_time_t, &time_buf);
    strftime(receive_time, 32, "%Y-%m-%d %H:%M:%S", &time_buf);
    return 0;
}

// 添加客户端连接信息到日志文件
bool datalog_save_connect_time(ClientNode *client, char *connect_state)
{
    pthread_mutex_lock(&datalog_mutex);
    if (client == NULL)
    {
        perror("client 为 NULL");
        return false;
    }
    // 示例：将 time_t 转为字符串
    char current_time[32];
    time_t current_time_t;
    if (strcmp(connect_state, "connect_success") == 0)
    {
        current_time_t = client->connect_time;
    }
    else
    {
        current_time_t = time(NULL);
    }
    get_standard_time(current_time, &current_time_t);
    FILE *file = fopen("datalog.txt", "a"); // 追加模式
    // 为每个字段指定固定宽度（可根据实际需求调整宽度值）
    const int IP_WIDTH = 18;
    const int SOCKID_WIDTH = 41;
    const int MSG_WIDTH = 20;
    const int TIME_WIDTH = 20;
    if (!file)
    {
        perror("无法打开文件");
        pthread_mutex_unlock(&datalog_mutex);
        return false;
    }
    fprintf(file, "%-*s %-*d  %-*s %-*s\n", IP_WIDTH, client->ip, SOCKID_WIDTH, client->sock_fd,
            MSG_WIDTH, connect_state, TIME_WIDTH, current_time);
    fclose(file);
    pthread_mutex_unlock(&datalog_mutex);
    memset(current_time, 0, 32);
    return true;
}
// 添加客户端操作信息到日志文件
bool datalog_save(ClientNode *client, const Packet *receive_packet)
{
    if (client == NULL)
    {
        perror("client 为 NULL");
        return false;
    }
    pthread_mutex_lock(&datalog_mutex);
    char send_message[1024] = {0};
    // 示例：将 time_t 转为字符串
    char current_time[32];
    time_t current_time_t = time(NULL);
    get_standard_time(current_time, &current_time_t);
    FILE *file = fopen("datalog.txt", "a"); // 追加模式
    // 为每个字段指定固定宽度（可根据实际需求调整宽度值）
    const int IP_WIDTH = 18;
    const int SOCKID_WIDTH = 10;
    const int USERNAME_WIDTH = 10;
    const int TYPE_WIDTH = 20;
    const int MSG_WIDTH = 20;
    const int TIME_WIDTH = 20;
    if (!file)
    {
        perror("无法打开文件");
        pthread_mutex_unlock(&datalog_mutex);
        return false;
    }
    if (receive_packet->data_type == TYPE_MSG)
    {
        fprintf(file, "%-*s %-*d %-*s %-*s %-*s %-*s\n",
                IP_WIDTH, client->ip, SOCKID_WIDTH, client->sock_fd,
                USERNAME_WIDTH, client->username,
                TYPE_WIDTH, enum_type_names[receive_packet->data_type],
                MSG_WIDTH, receive_packet->data + receive_packet->size,
                TIME_WIDTH, current_time);
    }
    else if (receive_packet->data_type == TYPE_HEART)
    { // 包太多不做处理
    }
    else
    {
        fprintf(file, "%-*s %-*d %-*s %-*s %-*s %-*s\n",
                IP_WIDTH, client->ip, SOCKID_WIDTH, client->sock_fd,
                USERNAME_WIDTH, client->username,
                TYPE_WIDTH, enum_type_names[receive_packet->data_type],
                MSG_WIDTH, receive_packet->data,
                TIME_WIDTH, current_time);
    }

    fclose(file);
    pthread_mutex_unlock(&datalog_mutex);
    return true;
}
// 获取枚举对应的名称
const char *get_enum_name(int enum_value)
{
    // 检查枚举值是否在有效范围内
    if (enum_value >= TYPE_LOGIN && enum_value <= TYPE_ERR)
    {
        return enum_type_names[enum_value];
    }
    return "UNKNOWN_TYPE"; // 未知类型
}
Packet reg_packet(const Packet *packet)
{
    Account acc = {0}; // 不太理解为什么acc会有缓存数据
    Packet relypack = {0};
    relypack.data_type = TYPE_REG;
    // printf("packet.data2=%s\n",(*packet).data);
    strncpy(acc.username, (*packet).data, (*packet).size);
    strcpy(acc.password, (*packet).data + (*packet).size);
    // printf("acc.username=%s\n",acc.username);
    // printf("acc.password=%s\n",acc.password);
    if (register_account("accouts.txt", acc.username, acc.password))
    {
        strcpy(relypack.data, "注册成功");
        relypack.data_type = TYPE_OK;
    }
    else
    {
        strcpy(relypack.data, "该用户名已被注册，重新选择用户名");
        relypack.data_type = TYPE_ERR;
    }
    relypack.size = strlen(relypack.data);
    return relypack;
}

Packet login_packet(const Packet *packet, const int *client_socket, const char *ip)
{
    Account acc = {0};
    Packet relypack = {0};
    relypack.data_type = TYPE_REG;
    // printf("packet.data2=%s\n",(*packet).data);
    strncpy(acc.username, (*packet).data, (*packet).size);
    strcpy(acc.password, (*packet).data + (*packet).size);
    // printf("acc.username=%s\n",acc.username);
    // printf("acc.password=%s\n",acc.password);
    ClientNode *node = client_list_username_find(client_list, acc.username);
    if (node != NULL)
    {
        strcpy(relypack.data, "该用户名已登录");
        relypack.data_type = TYPE_ERR;
    }
    else if (login_account("accouts.txt", acc.username, acc.password))
    {
        strcpy(relypack.data, "登录成功");
        client_login_insert(client_list, acc.username, client_socket);
        relypack.data_type = TYPE_OK;
    }
    else
    {
        strcpy(relypack.data, "登录失败，用户名或密码错误");
        relypack.data_type = TYPE_ERR;
    }
    relypack.size = strlen(relypack.data);
    return relypack;
}
Packet change_password_packet(const Packet *packet)
{

    Packet relypack = {0};
    char packet_accout_data[1024] = {0};
    strcpy(packet_accout_data, (*packet).data);
    relypack.data_type = TYPE_CHANGE_PASSWORD;
    // printf("packet.data2=%s\n",(*packet).data);
    char user_list[3][20];
    int user_count = 0;
    char *token = strtok(packet_accout_data, ","); // 分割字符串
    while (token != NULL)
    {
        strcpy(user_list[user_count], token);
        user_count++;
        token = strtok(NULL, ",");
    }
    if (modify_account("accouts.txt", user_list[0], user_list[1], user_list[2]))
    {
        strcpy(relypack.data, "修改成功");
        relypack.data_type = TYPE_OK;
    }
    else
    {
        strcpy(relypack.data, "修改失败,请检查用户名和旧密码是否正确");
        relypack.data_type = TYPE_ERR;
    }
    relypack.size = strlen(relypack.data);
    return relypack;
}
Packet cmd_packet(const Packet *packet)
{
    Packet relypack = {0};
    // printf("packet.data=%s\n",(*packet).data);
    FILE *fp = popen(packet->data, "r");
    if (fp == NULL)
    {
        perror("popen failed");
        relypack.data_type = TYPE_ERR;
        strcpy(relypack.data, "命令执行失败");
        return relypack;
    }

    char temp_buf[1024] = {0};

    relypack.data[0] = '\0';
    // 按行读取，直到文件结束
    while (fgets(temp_buf, sizeof(temp_buf), fp) != NULL)
    {
        if (strlen(relypack.data) + strlen(temp_buf) < sizeof(relypack.data))
        {
            strcat(relypack.data, temp_buf);
        }
        else
        {
            // 处理缓冲区即将溢出情况，如截断、报错等，这里简单示例截断提示
            strcat(relypack.data, "[输出内容过长已截断]");
            break;
        }
    }
    pclose(fp);
    relypack.data_type = TYPE_OK;
    relypack.size = strlen(relypack.data);
    return relypack;
}

Packet online_packet(const Packet *packet, const int *client_socket, const char *ip)
{
    Packet relypack = {0};
    if (strcmp(packet->data, "list_user") == 0) // 查询在线链表
    {
        client_logined_traverse(client_list, &relypack);
        // printf("online_packet:%s\n",relypack.data);
    }
    else
    { // 建立具体客户端的连接
    }
    relypack.data_type = TYPE_ONLINE;
    relypack.size = strlen(relypack.data);
    return relypack;
}
Packet logout_packet(const int *client_socket)
{
    Packet relypack = {0};
    if (client_logout_delete(client_list, client_socket) != 0)
    {
        strcpy(relypack.data, "账号登出成功！");
        relypack.data_type = TYPE_OK;
    }
    else
    {
        strcpy(relypack.data, "账号登出失败！");
        relypack.data_type = TYPE_ERR;
    }
    relypack.size = strlen(relypack.data);
    return relypack;
}

Packet packet_handle(const Packet *packet, const int *client_socket, const char *ip)
{
    Packet relypack = {0};
    switch ((*packet).data_type)
    {
    case TYPE_LOGIN:
        relypack = login_packet(packet, client_socket, ip);
        break;
    case TYPE_REG:
        relypack = reg_packet(packet);
        // printf("packet.data1=%s\n",(*packet).data);
        break;
    case TYPE_CHANGE_PASSWORD:
        relypack = change_password_packet(packet);
        break;
    case TYPE_MSG:
    {
        char sender_name[64];
        // 获取发送者用户名
        strcpy(sender_name, client_get_username(client_list, client_socket));
        relypack = send_messsage(packet, sender_name);
    }
    break;
    case TYPE_HEART:
        heart_count = 0; // 重置心跳次数
        relypack.data_type = TYPE_HEART;
        break;
    case TYPE_CMD:
        relypack = cmd_packet(packet);
        break;
    case TYPE_ONLINE:
        relypack = online_packet(packet, client_socket, ip);
        break;
    case TYPE_LOGOUT:
    {
        relypack = logout_packet(client_socket);
    }
    break;
    default:
        break;
    }
    ClientNode *current_node = client_list_socketid_find(client_list, client_socket);
    datalog_save(current_node, packet);
    return relypack;
}
Packet send_messsage(const Packet *packet, const char *sender_name)
{
    Packet receiver_packet = {0};
    Packet sender_packet = {0};
    char receiver_name[64];
    char send_message[100];
    // 拆包获取要转发的客户名
    strncpy(receiver_name, packet->data, packet->size);
    receiver_name[packet->size] = '\0'; // 确保接收者名以\0结尾
    // 拆包获取要转发的消息
    strcpy(send_message, packet->data + packet->size);
    // 封装转发的包
    sender_packet.data_type = TYPE_MSG;
    sender_packet.size = 0;
    // 让收到信息的客户端知道发送者是谁
    strcpy(sender_packet.data, sender_name);
    strcat(sender_packet.data, "发来消息:");
    strcat(sender_packet.data, send_message);
    // 添加信息日志
    char current_time[32];
    struct tm current_buf; // 用于存储当前时间的缓冲区
    time_t current_time_t = time(NULL);
    localtime_r(&current_time_t, &current_buf);
    strcat(sender_packet.data, "      ");
    strftime(current_time, sizeof(current_time), "%Y-%m-%d %H:%M:%S", &current_buf);
    strcat(sender_packet.data, current_time);
    strcat(sender_packet.data, "\0");

    sender_packet.size = strlen(sender_name);
    ClientNode *sender_node = client_list_username_find(client_list, sender_name);
    if (sender_node != NULL)
    {
        sender_node->send_count++;
    }

    // 转发消息
    ClientNode *receiver_node = client_list_username_find(client_list, receiver_name);
    if (receiver_node != NULL)
    {
        send(receiver_node->sock_fd, &sender_packet, sizeof(sender_packet), 0);

        receiver_packet.data_type = TYPE_OK;
        strcpy(receiver_packet.data, "信息发送成功");
        receiver_packet.size = strlen(receiver_packet.data);
        return receiver_packet;
    }
}