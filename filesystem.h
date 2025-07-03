#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define DISK_NAME "disk.img"
#define BLOCK_SIZE 512
#define INODE_SIZE 64
#define MAX_FILENAME 28
#define MAX_USERNAME 20
#define MAX_PASSWORD 20
#define MAX_INODES 128
#define MAX_BLOCKS 1024
#define MAX_USERS 16
#define MAX_OPEN_FILES 16

// 超级块结构
typedef struct {
    int total_blocks;
    int free_blocks;
    int total_inodes;
    int free_inodes;
    int inode_bitmap_offset;
    int block_bitmap_offset;
    int inode_table_offset;
    int data_blocks_offset;
} SuperBlock;

// Inode结构
typedef struct {
    char filename[MAX_FILENAME];
    int owner_id;
    char permission[4];
    int size;
    int data_blocks[8];    // 支持最多8个块
    int block_count;       // 已分配块数
    time_t created;
    time_t modified;
    time_t accessed;
    bool is_directory;
    int parent_inode;
} Inode;


// 用户结构
typedef struct {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    int uid;
} User;

// 打开文件表项
typedef struct {
    int inode_id;
    int position;
    bool is_open;
} OpenFile;

// 文件系统结构
typedef struct {
    SuperBlock super_block;
    char inode_bitmap[MAX_INODES/8];
    char block_bitmap[MAX_BLOCKS/8];
    Inode inode_table[MAX_INODES];
    User users[MAX_USERS];
    int user_count;
    int current_user;
    int current_dir;
    OpenFile open_files[MAX_OPEN_FILES];
    FILE* disk;
} FileSystem;

// 函数声明
int move_file(char* oldname, char* newname);
int change_password(char* oldpass, char* newpass);

void init_filesystem();
void load_filesystem();
void save_filesystem();
void format_disk();
int login(char* username, char* password);
void logout();
void list_directory();
int create_file(char* filename, bool is_directory);
int delete_file(char* filename);
int open_file(char* filename);
void close_file(int fd);
int read_file(int fd, char* buffer, int size);
int write_file(int fd, char* buffer, int size);
int change_permission(char* filename, char* new_perm);
void exit_system();
int change_directory(char* dirname);
int copy_file(char* source, char* destination);
int create_user(char* username, char* password);
void show_file_info(char* filename);
int append_file(char* filename, char* content);
// 文件系统一致性检查
int check_filesystem();

// 辅助函数
int allocate_inode();
int allocate_block();
void free_inode(int inode_id);
void free_block(int block_id);
int find_file(char* filename);
bool check_permission(int inode_id, char required_perm);

// 文件搜索
void search_file(char* pattern);

// 文件重命名
int rename_file(char* oldname, char* newname);

// 磁盘使用情况统计
void disk_usage();

#endif
