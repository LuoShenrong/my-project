#include "client_agreement.h"
#include <stdlib.h>

#ifndef __CLIENT_UI_H__
#define __CLIENT_UI_H__

void client_register(const int *socketid);
void client_ui(const int *socketid);
void client_login(const int *socketid);
void accout_logout(const int *socketid);
void client_logined(const int *socketid);
void shell_exec(const int *socketid);
void list_user(const int *socketid);
void chat(const char *username, const int *socketid);
Packet packet_send(const int *socketid, const Packet *send_packet);
Packet packet_receive(const int *socketid);

#endif
