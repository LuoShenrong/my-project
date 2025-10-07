#include "client_agreement.h"
Packet create_reg_packet(const char *username, const char *password)
{
    Packet packet;
    packet.data_type = TYPE_REG;
    packet.size = 0;
    strcpy(packet.data, username);
    strcpy(packet.data + strlen(username), password);
    packet.data[strlen(username) + strlen(password) + 1] = '\0';
    packet.size = strlen(username);
    return packet;
}
Packet create_login_packet(const char *username, const char *password)
{
    Packet packet;
    packet.data_type = TYPE_LOGIN;
    packet.size = 0;
    strcpy(packet.data, username);
    strcpy(packet.data + strlen(username), password);
    packet.data[strlen(username) + strlen(password) + 1] = '\0';
    packet.size = strlen(username);
    return packet;
}

Packet create_change_password_packet(const char *username, const char *oldpassword, const char *newpassword)
{
    Packet packet;
    packet.data_type = TYPE_CHANGE_PASSWORD;
    packet.size = 0;
    strcpy(packet.data, username);
    strcat(packet.data, ",");
    strcat(packet.data, oldpassword);
    strcat(packet.data, ",");
    strcat(packet.data, newpassword);
    strcat(packet.data, "\0");
    packet.size = strlen(packet.data);
    return packet;
}

Packet create_cmd_packet(const char *cmd)
{
    Packet packet;
    packet.data_type = TYPE_CMD;
    packet.size = 0;
    strcpy(packet.data, cmd);
    packet.data[strlen(cmd) + 1] = '\0';
    packet.size = strlen(cmd);
    return packet;
}
Packet create_msg_packet(const char *username, const char *message)
{
    Packet packet;
    packet.data_type = TYPE_MSG;
    packet.size = 0;
    strcpy(packet.data, username);
    strcpy(packet.data + strlen(username), message);
    packet.data[strlen(username) + strlen(message) + 1] = '\0';
    packet.size = strlen(username);
    return packet;
}
