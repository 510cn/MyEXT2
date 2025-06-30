#include "filesystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COMMAND_LENGTH 100
#define MAX_ARGS 10

// 解析命令行输入
void parse_command(char* input, char* command, char* args[], int* arg_count) {
    char* token;
    *arg_count = 0;
    
    // 提取命令
    token = strtok(input, " \n");
    if (token == NULL) {
        command[0] = '\0';
        return;
    }
    strcpy(command, token);
    
    // 提取参数
    while ((token = strtok(NULL, " \n")) != NULL && *arg_count < MAX_ARGS) {
        args[*arg_count] = (char*)malloc(strlen(token) + 1);
        strcpy(args[*arg_count], token);
        (*arg_count)++;
    }
}

// 释放参数内存
void free_args(char* args[], int arg_count) {
    for (int i = 0; i < arg_count; i++) {
        free(args[i]);
    }
}

// 显示帮助信息
void show_help() {
    printf("\n可用命令：\n");
    printf("login <username> <password> - 用户登录\n");
    printf("logout - 用户登出\n");
    printf("dir - 列出当前目录内容\n");
    printf("cd <dirname> - 切换目录\n");
    printf("create <filename> - 创建文件\n");
    printf("mkdir <dirname> - 创建目录\n");
    printf("delete <filename> - 删除文件或目录\n");
    printf("copy <source> <destination> - 复制文件\n");
    printf("rename <oldname> <newname> - 重命名文件\n");
    printf("open <filename> - 打开文件\n");
    printf("close <fd> - 关闭文件\n");
    printf("read <fd> <size> - 读取文件内容\n");
    printf("write <fd> <content> - 写入文件内容\n");
    printf("append <filename> <content> - 追加内容到文件\n");
    printf("chmod <filename> <permissions> - 修改文件权限\n");
    printf("adduser <username> <password> - 创建新用户(仅root)\n");
    printf("find <pattern> - 搜索文件\n");
    printf("fsck - 文件系统一致性检查\n");
    printf("du - 显示磁盘使用情况\n");
    printf("exit - 退出系统\n");
    printf("help - 显示帮助信息\n\n");
}

int main() {
    char input[MAX_COMMAND_LENGTH];
    char command[MAX_COMMAND_LENGTH];
    char* args[MAX_ARGS];
    int arg_count;
    char buffer[BLOCK_SIZE];
    
    printf("欢迎使用Linux二级文件系统模拟器！\n");
    printf("输入 'help' 获取命令列表。\n\n");
    
    // 初始化文件系统
    init_filesystem();
    
    while (1) {
        printf("fs> ");
        if (fgets(input, MAX_COMMAND_LENGTH, stdin) == NULL) {
            break;
        }
        
        parse_command(input, command, args, &arg_count);
        
        if (strlen(command) == 0) {
            continue;
        }
        
        // 处理命令
        if (strcmp(command, "help") == 0) {
            show_help();
        }
        else if (strcmp(command, "login") == 0) {
            if (arg_count < 2) {
                printf("用法: login <username> <password>\n");
            } else {
                login(args[0], args[1]);
            }
        }
        else if (strcmp(command, "logout") == 0) {
            logout();
        }
        else if (strcmp(command, "dir") == 0) {
            list_directory();
        }
        else if (strcmp(command, "cd") == 0) {
            if (arg_count < 1) {
                printf("用法: cd <dirname>\n");
            } else {
                change_directory(args[0]);
            }
        }
        else if (strcmp(command, "create") == 0) {
            if (arg_count < 1) {
                printf("用法: create <filename>\n");
            } else {
                create_file(args[0], false);
            }
        }
        else if (strcmp(command, "mkdir") == 0) {
            if (arg_count < 1) {
                printf("用法: mkdir <dirname>\n");
            } else {
                create_file(args[0], true);
            }
        }
        else if (strcmp(command, "delete") == 0) {
            if (arg_count < 1) {
                printf("用法: delete <filename>\n");
            } else {
                delete_file(args[0]);
            }
        }
        else if (strcmp(command, "copy") == 0) {
            if (arg_count < 2) {
                printf("用法: copy <source> <destination>\n");
            } else {
                copy_file(args[0], args[1]);
            }
        }
        else if (strcmp(command, "open") == 0) {
            if (arg_count < 1) {
                printf("用法: open <filename>\n");
            } else {
                open_file(args[0]);
            }
        }
        else if (strcmp(command, "close") == 0) {
            if (arg_count < 1) {
                printf("用法: close <fd>\n");
            } else {
                close_file(atoi(args[0]));
            }
        }
        else if (strcmp(command, "read") == 0) {
            if (arg_count < 2) {
                printf("用法: read <fd> <size>\n");
            } else {
                int fd = atoi(args[0]);
                int size = atoi(args[1]);
                if (size > BLOCK_SIZE) {
                    size = BLOCK_SIZE;
                    printf("读取大小已调整为最大块大小: %d\n", BLOCK_SIZE);
                }
                
                memset(buffer, 0, BLOCK_SIZE);
                int bytes_read = read_file(fd, buffer, size);
                if (bytes_read > 0) {
                    printf("读取了 %d 字节：\n", bytes_read);
                    buffer[bytes_read] = '\0'; // 确保字符串结束
                    printf("%s\n", buffer);
                }
            }
        }
        else if (strcmp(command, "write") == 0) {
            if (arg_count < 2) {
                printf("用法: write <fd> <content>\n");
            } else {
                int fd = atoi(args[0]);
                int bytes_written = write_file(fd, args[1], strlen(args[1]));
                if (bytes_written > 0) {
                    printf("成功写入 %d 字节。\n", bytes_written);
                }
            }
        }
        else if (strcmp(command, "chmod") == 0) {
            if (arg_count < 2) {
                printf("用法: chmod <filename> <permissions>\n");
            } else {
                change_permission(args[0], args[1]);
            }
        }
        else if (strcmp(command, "adduser") == 0) {
            if (arg_count < 2) {
                printf("用法: adduser <username> <password>\n");
            } else {
                create_user(args[0], args[1]);
            }
        }
        else if (strcmp(command, "exit") == 0) {
            exit_system();
            break;
        }
        else if (strcmp(command, "stat") == 0) {
            if (arg_count < 1) {
                printf("用法: stat <filename>\n");
            } else {
                show_file_info(args[0]);
            }
        }
        else if (strcmp(command, "append") == 0) {
            if (arg_count < 2) {
                printf("用法: append <filename> <content>\n");
            } else {
                append_file(args[0], args[1]);
            }
        }
        else if (strcmp(command, "rename") == 0) {
            if (arg_count < 2) {
                printf("用法: rename <oldname> <newname>\n");
            } else {
                rename_file(args[0], args[1]);
            }
        }
        else if (strcmp(command, "find") == 0) {
            if (arg_count < 1) {
                printf("用法: find <pattern>\n");
            } else {
                search_file(args[0]);
            }
        }
        else if (strcmp(command, "fsck") == 0) {
            check_filesystem();
        }
        else if (strcmp(command, "du") == 0) {
            disk_usage();
        }
        else {
            printf("未知命令: %s\n", command);
            printf("输入 'help' 获取命令列表。\n");
        }
        
        free_args(args, arg_count);
    }
    
    return 0;
}
