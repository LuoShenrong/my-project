
#include <stdio.h>
#include <string.h>
// 数据包结构，与服务器端对应
#include "client_ui.h"
#include <pthread.h>
void *heart_send(void *arg) // 子线程函数
{
    int socketid = (int)(long)arg;
    Packet heart_packet;
    heart_packet.data_type = TYPE_HEART;
    heart_packet.size = 0;
    heart_packet.data[0] = '\0';
    while (1)
    {
        sleep(3);
        packet_send(&socketid, &heart_packet);
    }
    return NULL;
}
int main()
{
    int socketid = socket(AF_INET, SOCK_STREAM, 0);
    if (socketid < 0)
    {
        perror("socket");
        return 1; // Error creating socket
    }
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("192.168.191.130");
    addr.sin_port = htons(9000);
    if (connect(socketid, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("connect");
        close(socketid);
        return 1; // Error connecting to server
    }
    pthread_t tid = 0;
    if (pthread_create(&tid, NULL, heart_send, (void *)(long)socketid) < 0) // 创建子线程
    {
        perror("子线程开启失败");
        close(socketid);
        pthread_cancel(tid); // 取消线程
        pthread_detach(tid); // 分离线程
        return 1;
    }
    client_ui(&socketid);
    return 0; // Success
}