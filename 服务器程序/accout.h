#ifndef __ACCOUT_H__
#define __ACCOUT_H__
#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

// 账号密码结构体
typedef struct
{
    char username[50];
    char password[50];
} Account;
extern pthread_rwlock_t accout_rwlock;
// 从文件读取所有账号
bool load_accounts(const char *filename, Account **accounts, int *count);
// 添加新账号到文件
bool save_account(const char *filename, const char *username, const char *password);
// 注册账号
bool register_account(const char *filename, const char *username, const char *password);
// 修改密码
bool modify_account(const char *filename, const char *username, const char *oldPassword, const char *newPassword);
// 登录
bool login_account(const char *filename, const char *username, const char *Password);

#endif