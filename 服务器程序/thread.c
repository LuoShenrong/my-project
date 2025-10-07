#include "thread.h"
extern ClientList *client_list;
int heart_count = 0;
extern int server_state;
void *heart_deal(void *arg)
{

    int client_socket = (int)(long)arg;
    while (1)
    {
        sleep(3);
        heart_count++;
        if (heart_count >= 5)
        {
            // 判断客户端是否已经被线程识别到断开并删除了节点
            if (client_list_socketid_find(client_list, &client_socket) != NULL)
            {
                ClientNode *connecting_node = client_list_socketid_find(client_list, &client_socket);
                datalog_save_connect_time(connecting_node, "心跳超时");
                client_list_delete(client_list, &client_socket);
                close(client_socket);
            }
            break;
        }
    }
}
void *handle_client(void *arg)
{
    ThreadArgs *client_info = (ThreadArgs *)arg;
    int client_socket = client_info->client_socket;
    char *ip = client_info->ip;
    // 客户端插入
    ClientNode *connecting_node = client_list_insert(client_list, ip, &client_socket);
    datalog_save_connect_time(connecting_node, "connect_success");
    pthread_t heart_tid;
    // 子线程中创建孙线程
    int ret = pthread_create(&heart_tid, NULL, heart_deal, (void *)(long)client_socket);
    pthread_detach(heart_tid);
    if (ret != 0)
    {
        printf("创建子线程失败\n");
        return NULL;
    }
    Packet packet;
    Packet relypack;
    while (1)
    {

        memset(&packet, 0, sizeof(packet)); // 清空packet
        memset(&relypack, 0, sizeof(relypack));
        ssize_t n = recv(client_socket, &packet, sizeof(packet), 0);
        // printf("收到客户端:%d 消息:%s,data_type:%s\n", client_socket,packet.data,get_enum_name(packet.data_type));
        if (n < 0)
        {
            perror("recv");
            strcpy(relypack.data, "信息接收失败，请重试！");
            relypack.data_type = TYPE_ERR;
            relypack.size = strlen(relypack.data);
            send(client_socket, &relypack, sizeof(relypack), 0);
            break; // Error receiving data
        }
        if (n == 0)
        {
            // printf("Client disconnected\n");
            datalog_save_connect_time(connecting_node, "disconnect");
            client_list_delete(client_list, &client_socket);
            close(client_socket);
            break; // Client disconnected
        }

        relypack = packet_handle(&packet, &client_socket, ip);

        // Echo the received data back to the client
        if (relypack.data_type != TYPE_HEART)
        {
            send(client_socket, &relypack, sizeof(relypack), 0);
            //  printf("发回客户端:%d 消息:%s,data_type:%s\n", client_socket,relypack.data,get_enum_name(relypack.data_type));
        }

        // close(client_socket); // Close the client socket after handling
    }
    // free(client_info);
    // pthread_cancel(heart_tid);
    return NULL;
}