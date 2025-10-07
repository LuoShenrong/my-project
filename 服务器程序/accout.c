#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "accout.h"

// 添加账号到文件
bool save_account(const char *filename, const char *username, const char *password)
{
    pthread_rwlock_wrlock(&accout_rwlock);
    FILE *file = fopen(filename, "a"); // 追加模式
    if (!file)
    {
        perror("无法打开文件");
        pthread_rwlock_unlock(&accout_rwlock);
        return false;
    }
    // 格式：用户名 密码（用空格分隔）
    fprintf(file, "%s %s\n", username, password);
    fclose(file);
    pthread_rwlock_unlock(&accout_rwlock);
    return true;
}

// 从文件读取所有账号
bool load_accounts(const char *filename, Account **accounts, int *count)
{
    pthread_rwlock_rdlock(&accout_rwlock);
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("无法打开文件");
        pthread_rwlock_unlock(&accout_rwlock);
        return false;
    }
    // 读取数据并动态扩容
    while (1)
    {
        Account temp_acc = {0};
        int result = fscanf(file, "%s %s", temp_acc.username, temp_acc.password);
        if (result != 2)
        {
            break;
        }
        // 先扩容，再赋值
        *count += 1;
        Account *temp = realloc(*accounts, *count * sizeof(Account));
        if (!temp)
        {
            free(*accounts);
            *accounts = NULL;
            *count = 0; // 回退 count，避免污染
            fclose(file);
            return false;
        }
        *accounts = temp;
        (*accounts)[*count - 1] = temp_acc;
    }
    // 检查退出循环的原因
    if (feof(file))
    {
        // printf("已成功读取到文件末尾，共读取 %d 条记录\n", *count);
    }
    else
    {
        // printf("读取文件时发生错误，但已成功读取 %d 条记录\n", *count);
    }
    // 关闭文件
    fclose(file);
    pthread_rwlock_unlock(&accout_rwlock);
    return true;
}

// 注册
bool register_account(const char *filename, const char *username, const char *password)
{
    int count = 0;
    Account *accounts = (Account *)malloc(sizeof(Account)); // 根据总数动态分配内存
    if (accounts == NULL)
    {
        perror("内存分配失败");
        return 1;
    }
    if (load_accounts(filename, &accounts, &count)) // 加载账号文件
    {
        for (int i = 0; i < count; i++)
        {
            if (strcmp(accounts[i].username, username) == 0)
            { // 账号已存在，更换账号名
                free(accounts);
                return false;
            }
            //  printf("accounts[%d].username:%s,username:%s\n",i,accounts[i].username,username);
        }
        if (save_account(filename, username, password)) // 添加账号
        {
            free(accounts);
            accounts = NULL; // 避免野指针
            return true;
        }
    }
    free(accounts);
    accounts = NULL; // 避免野指针
    return false;
}

// 修改账号密码
bool modify_account(const char *filename, const char *username, const char *oldPassword, const char *newPassword)
{

    int count = 0;                                          // 定义变量要初始化，否则会随机值，同时有被堆内存污染导致值异常的风险
    Account *accounts = (Account *)malloc(sizeof(Account)); // 根据总数动态分配内存
    if (accounts == NULL)
    {
        perror("内存分配失败");
        return false;
    }
    if (load_accounts(filename, &accounts, &count)) // 加载账号文件
    {
        for (int i = 0; i < count; i++)
        {
            if (strcmp(accounts[i].username, username) == 0)
            { // 查找账号名
                if (strcmp(accounts[i].password, oldPassword) == 0)
                {
                    strcpy(accounts[i].password, newPassword);
                    accounts[i].password[strlen(newPassword)] = '\0'; // 添加字符串结束符
                                                                      //  重新写入所有账号
                    pthread_rwlock_wrlock(&accout_rwlock);
                    FILE *file = fopen(filename, "w");
                    if (!file)
                    {
                        perror("无法打开文件");
                        pthread_rwlock_unlock(&accout_rwlock);
                        return false;
                    }

                    for (int j = 0; j < count; j++)
                    {
                        fprintf(file, "%s %s\n", accounts[j].username, accounts[j].password);
                    }
                    fclose(file);
                    pthread_rwlock_unlock(&accout_rwlock);
                    free(accounts);
                    accounts = NULL; // 避免野指针
                    return true;
                }
                else
                {
                    free(accounts);
                    accounts = NULL; // 避免野指针
                    return false;
                }
            }
        }
    }
    free(accounts);
    accounts = NULL; // 避免野指针
    return false;
}

// 登录
bool login_account(const char *filename, const char *username, const char *Password)
{

    int count = 0;
    // printf("haha4");
    Account *accounts = (Account *)malloc(sizeof(Account)); // 根据总数动态分配内存
    if (accounts == NULL)
    {
        perror("内存分配失败");
        free(accounts);
        accounts = NULL;
        return false;
    }
    // printf("haha5");
    if (load_accounts(filename, &accounts, &count)) // 加载账号文件
    {
        for (int i = 0; i < count; i++)
        {
            if (strcmp(accounts[i].username, username) == 0 && strcmp(accounts[i].password,
                                                                      Password) == 0)
            { //
                free(accounts);
                accounts = NULL; // 避免野指针
                return true;
            }
        }
    }
    free(accounts);
    accounts = NULL; // 避免野指针
    return false;
}
