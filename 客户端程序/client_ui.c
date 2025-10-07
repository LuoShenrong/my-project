#include <stdio.h>
#include "client_ui.h"
#include "client_agreement.h"
#include <string.h>
static int login_count = 0; // 登录次数，不能超过三次
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
void client_register(const int *socketid)
{
    char username[50];
    char password[50];
    printf("请输入用户名：");
    scanf("%s", username);
    printf("请输入密码：");
    scanf("%s", password);
    Packet reg_packet = create_reg_packet(username, password);
    packet_send(socketid, &reg_packet);
    Packet register_receive_packet = packet_receive(socketid);
    if (register_receive_packet.data_type == TYPE_OK)
    {
        printf("%s\n\n", register_receive_packet.data);
        client_login(socketid);
    }
    else if (register_receive_packet.data_type == TYPE_ERR)
    {
        printf("%s\n", register_receive_packet.data);
        client_register(socketid);
    }
}
void client_login(const int *socketid)
{
    char username[50];
    char password[50];
    printf("请输入用户名：");
    scanf("%s", username);
    printf("请输入密码：");
    scanf("%s", password);
    Packet login_packet = create_login_packet(username, password);
    packet_send(socketid, &login_packet);
    Packet login_receive_packet = packet_receive(socketid);
    if (login_receive_packet.data_type == TYPE_OK)
    {
        printf("%s\n\n", login_receive_packet.data);
        login_count = 0;
        client_logined(socketid);
    }
    else if (login_receive_packet.data_type == TYPE_ERR)
    {
        printf("%s\n", login_receive_packet.data);
        login_count++;
        if (login_count >= 3)
        {
            printf("登录失败，用户名或密码错误3次，退出程序\n");
            close(*socketid);
            exit(0);
        }
        client_login(socketid);
    }
}
Packet packet_send(const int *socketid, const Packet *send_packet)
{
    ssize_t n;
    // memset(send_packet, 0, sizeof(*send_packet));
    // memset(&recv_packet, 0, sizeof(recv_packet));
    n = send(*socketid, send_packet, sizeof(*send_packet), 0);
    if (n < 0)
    {
        perror("send");
        close(*socketid);
        exit(0);
    }
    // if(send_packet->data_type!=TYPE_HEART)
    // {
    //      printf("send %ld bytes ：%sto server，data_type：%s\n",n,send_packet->data,get_enum_name(send_packet->data_type));
    // }
}
Packet packet_receive(const int *socketid)
{
    // Packet ack_packet;
    Packet recv_packet;
    ssize_t n;
    // memset(&ack_packet, 0, sizeof(ack_packet));
    memset(&recv_packet, 0, sizeof(recv_packet));
    n = recv(*socketid, &recv_packet, sizeof(recv_packet), 0);
    // printf("收到服务器消息:%s,data_type:%s\n",recv_packet.data,get_enum_name(recv_packet.data_type));
    if (n < 0)
    {
        perror("recv");
        close(*socketid);
        exit(0); // Error receiving data
    }
    if (n == 0)
    { // 对端关闭连接
        printf("Server closed the connection\n");
        // Server closed the connection
    }

    while (recv_packet.data_type == TYPE_MSG)
    {
        printf("%s\n", recv_packet.data);
        //    ack_packet.data_type=TYPE_OK;
        //    packet_send(socketid,&ack_packet);
        memset(&recv_packet, 0, sizeof(recv_packet));
        n = recv(*socketid, &recv_packet, sizeof(recv_packet), 0);
        if (n < 0)
        {
            perror("recv");
            close(*socketid);
            exit(0); // Error receiving data
        }
        if (n == 0)
        { // 对端关闭连接
            printf("Server closed the connection\n");
            // Server closed the connection
        }
        // printf("收到服务器消息:%s,data_type:%s\n",recv_packet.data,get_enum_name(recv_packet.data_type));
    }

    return recv_packet;
}

void change_password(const int *socketid)
{
    char username[50];
    char newpassword[50];
    printf("请输入用户名：");
    scanf("%s", username);
    printf("请输入旧密码：");
    char oldpassword[50];
    scanf("%s", oldpassword);
    printf("密码由字母、数字和下划线组成，长度在6-16之间\n请输入新密码：");
    scanf("%s", newpassword);

    Packet change_password_packet = create_change_password_packet(username, oldpassword, newpassword);
    packet_send(socketid, &change_password_packet);
    Packet change_password_receive_packet = packet_receive(socketid);
    if (change_password_receive_packet.data_type == TYPE_OK)
    {
        printf("%s\n\n", change_password_receive_packet.data);

        client_login(socketid);
    }
    else
    {
        printf("%s\n", change_password_receive_packet.data);
        change_password(socketid);
    }
}
void client_ui(const int *socketid)
{
    printf("1.注册\n");
    printf("2.登录\n");
    printf("3.修改密码\n");
    printf("4.退出\n");
    printf("请输入您的选择：");
    int choice;

    scanf("%d", &choice);
    switch (choice)
    {
    case 1:
        client_register(socketid);
        break;
    case 2:
        client_login(socketid);
        break;
    case 3:
        change_password(socketid);
        break;
    case 4:
        printf("退出\n");
        close(*socketid);
        exit(0);
        break;
    default:
        printf("无效选择\n");
        int c;
        while ((c = getchar()) != '\n' && c != EOF)
            ;
        client_ui(socketid);
        // 清除输入缓冲区中残留的换行符

        break;
    };
}

void client_logined(const int *socketid)
{
    char choice[10];
    printf("当前客户端有以下功能可选，请输入要选择的选项指令\n");
    printf("1.exec-sh\n");
    printf("2.list-user\n");
    printf("3.退出登录\n");
    scanf("%s", choice);
    // 清除输入缓冲区中残留的换行符
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
    if (strcmp(choice, "1") == 0)
    {
        shell_exec(socketid);
    }
    else if (strcmp(choice, "2") == 0)
    {
        list_user(socketid);
    }
    else if (strcmp(choice, "3") == 0)
    {

        accout_logout(socketid);
    }
    else
    {
        printf("无效选择\n");
        client_logined(socketid);
    }
}
void shell_exec(const int *socketid)
{
    char cmd[100];
    printf("输入exit可退出shell_exec模式\n");
    printf("请输入要执行的shell命令：");
    if (fgets(cmd, sizeof(cmd), stdin) == NULL) // 用scanf,读到空格就不读了，命令缺失
    {
        perror("读取命令失败");
        shell_exec(socketid);
        return;
    }
    // 移除末尾的换行符（如果存在）
    // 例如输入"ls -l\n"，会被处理为"ls -l"
    size_t len = strlen(cmd);
    if (len > 0 && cmd[len - 1] == '\n')
    {
        cmd[len - 1] = '\0';
    }
    if (strcmp(cmd, "exit") == 0)
    {
        printf("\n");
        client_logined(socketid);
    }

    else
    {
        Packet cmd_packet = create_cmd_packet(cmd);
        packet_send(socketid, &cmd_packet);
        Packet cmd_receive_packet = packet_receive(socketid);
        printf("%s\n", cmd_receive_packet.data);
        shell_exec(socketid);
    }
}

void list_user(const int *socketid)
{

    Packet list_user_packet;
    list_user_packet.data_type = TYPE_ONLINE;
    strcpy(list_user_packet.data, "list_user");
    packet_send(socketid, &list_user_packet);
    Packet receive_packet = packet_receive(socketid);
    // printf("%s\n",receive_packet.data);
    char user_list[5][20];
    int user_count = 0;
    char *token = strtok(receive_packet.data, ","); // 分割字符串
    while (token != NULL)
    {
        strcpy(user_list[user_count], token);
        user_count++;
        token = strtok(NULL, ",");
    }
    for (int i = 0; i < user_count; i++)
    {
        printf("客户端%d：%s\n", i + 1, user_list[i]);
    }
    char choice[10];
    int num = 0;
    while (1)
    {
        printf("请输入要聊天的客户端编号：");
        memset(&choice, 0, sizeof(choice));
        scanf("%s", choice);
        if (strcmp(choice, "exit") == 0)
        {
            break;
        }
        num = choice[0] - '0';
        if (num <= 0 || num > user_count)
        {
            printf("选择错误，请重新选择\n");
            continue;
        }
        else
        {
            break;
        }
    }
    if (strcmp(choice, "exit") == 0)
    {
        printf("\n");
        client_logined(socketid);
    }
    else
    {
        chat(user_list[num - 1], socketid);
    }
}

void chat(const char *username, const int *socketid)
{
    char message[100] = "";
    int c;
    // 清除输入缓冲区残留的换行符
    while ((c = getchar()) != '\n' && c != EOF)
        ;
    Packet message_packet, message_receive_packet;
    while (1)
    { // 用循环替代递归，持续收发消息
        memset(message, 0, sizeof(message));
        memset(&message_packet, 0, sizeof(message_packet));
        memset(&message_receive_packet, 0, sizeof(message_receive_packet));
        printf("\n输入exit可退出聊天：\n");
        printf("请输入要发送的消息：");
        if (fgets(message, sizeof(message), stdin) == NULL)
        {
            perror("读取消息失败");
            continue;
        }
        // 移除末尾的换行符
        size_t len = strlen(message);
        if (len > 0 && message[len - 1] == '\n')
        {
            message[len - 1] = '\0';
        }
        if (strcmp(message, "exit") == 0)
        {
            printf("\n");
            list_user(socketid);
            break; // 退出循环，结束chat函数
        }
        else
        {
            message_packet = create_msg_packet(username, message);
            printf("发送消息给%s：%s\n", username, message_packet.data + message_packet.size);
            packet_send(socketid, &message_packet);
            message_receive_packet = packet_receive(socketid);
            printf("%s\n", message_receive_packet.data);
        }
    }
}

void accout_logout(const int *socketid)
{
    Packet logout_packet;
    logout_packet.data_type = TYPE_LOGOUT;
    strcpy(logout_packet.data, "");
    packet_send(socketid, &logout_packet);
    Packet receive_packet = packet_receive(socketid);
    printf("%s\n", receive_packet.data);
    if (receive_packet.data_type == TYPE_OK)
    {
        client_ui(socketid);
    }
    else
    {
        client_logined(socketid);
    }
}