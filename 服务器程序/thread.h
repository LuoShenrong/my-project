#ifndef __THREAD_H__
#define __THREAD_H__
#include <pthread.h>
#include "client_list.h"
#include "server_agreement.h"
void *handle_client(void *arg);

#endif
