/*
 * ============================================================================
 * 文件名: include/ext2fs.h
 * 描述: EXT2文件系统公共头文件
 * 功能: 定义所有模块共享的数据结构、常量和接口
 * 版本: 4.0 (模块化架构)
 * ============================================================================
 */

#ifndef EXT2FS_H
#define EXT2FS_H

// ============================================================================
// 系统头文件包含
// ============================================================================
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <unistd.h>

// 确保文件类型常量被定义
#ifndef S_IFDIR
#define S_IFDIR 0040000
#endif
#ifndef S_IFREG
#define S_IFREG 0100000
#endif

// ============================================================================
// 文件系统常量定义
// ============================================================================
#define EXT2FS_VERSION "4.0"
#define DISK_IMAGE "disk.img"           // 虚拟磁盘文件名
#define BLOCK_SIZE 512                  // 数据块大小
#define INODE_SIZE 64                   // inode结构大小
#define MAX_FILENAME 28                 // 最大文件名长度
#define MAX_USERNAME 20                 // 最大用户名长度
#define MAX_PASSWORD 20                 // 最大密码长度
#define MAX_INODES 128                  // 最大inode数量
#define MAX_BLOCKS 1024                 // 最大数据块数量
#define MAX_USERS 16                    // 最大用户数量
#define MAX_DIRECT_BLOCKS 8             // 最大直接数据块数

// 特殊inode编号
#define ROOT_INODE 0                    // 根目录inode编号
#define INVALID_INODE -1                // 无效inode编号
#define INVALID_BLOCK -1                // 无效块编号

// ============================================================================
// 核心数据结构定义
// ============================================================================

/**
 * 超级块 - 文件系统元数据
 */
typedef struct {
    uint32_t magic;                     // 魔数，用于识别文件系统
    uint32_t total_blocks;              // 总数据块数
    uint32_t free_blocks;               // 空闲数据块数
    uint32_t total_inodes;              // 总inode数
    uint32_t free_inodes;               // 空闲inode数
    uint32_t block_size;                // 数据块大小
    uint32_t inode_size;                // inode大小
    uint32_t inode_bitmap_offset;       // inode位图偏移
    uint32_t block_bitmap_offset;       // 数据块位图偏移
    uint32_t inode_table_offset;        // inode表偏移
    uint32_t data_blocks_offset;        // 数据块区域偏移
    time_t created;                     // 文件系统创建时间
    time_t last_mount;                  // 最后挂载时间
    uint32_t mount_count;               // 挂载次数
} SuperBlock;

/**
 * 索引节点 - 文件/目录元数据
 */
typedef struct {
    char name[MAX_FILENAME];            // 文件名
    uint32_t owner_id;                  // 所有者ID
    char permissions[4];                // 权限字符串 (rwx)
    uint32_t size;                      // 文件大小
    uint32_t data_blocks[MAX_DIRECT_BLOCKS]; // 直接数据块索引
    uint32_t block_count;               // 已分配块数
    time_t created;                     // 创建时间
    time_t modified;                    // 修改时间
    time_t accessed;                    // 访问时间
    uint8_t is_directory;               // 是否为目录
    uint32_t parent_inode;              // 父目录inode
    uint32_t link_count;                // 硬链接计数
    uint32_t reserved[2];               // 保留字段
} Inode;

/**
 * 用户信息
 */
typedef struct {
    char username[MAX_USERNAME];        // 用户名
    char password[MAX_PASSWORD];        // 密码
    uint32_t uid;                       // 用户ID
    uint32_t gid;                       // 组ID
    time_t created;                     // 创建时间
    time_t last_login;                  // 最后登录时间
} User;

/**
 * 目录项
 */
typedef struct {
    uint32_t inode;                     // 指向的inode编号
    uint16_t rec_len;                   // 记录长度
    uint8_t name_len;                   // 文件名长度
    uint8_t file_type;                  // 文件类型
    char name[];                        // 文件名 (变长)
} DirEntry;

/**
 * 文件系统实例
 */
typedef struct {
    SuperBlock superblock;              // 超级块
    char *inode_bitmap;                 // inode位图
    char *block_bitmap;                 // 数据块位图
    Inode *inode_table;                 // inode表
    User users[MAX_USERS];              // 用户表
    uint32_t user_count;                // 用户数量
    FILE *disk_file;                    // 磁盘文件句柄
    bool is_mounted;                    // 是否已挂载
    bool is_dirty;                      // 是否有未保存的更改
} FileSystem;

// ============================================================================
// 错误码定义
// ============================================================================
typedef enum {
    EXT2FS_SUCCESS = 0,                 // 成功
    EXT2FS_ERROR_GENERIC = -1,          // 通用错误
    EXT2FS_ERROR_NO_SPACE = -2,         // 空间不足
    EXT2FS_ERROR_NOT_FOUND = -3,        // 文件不存在
    EXT2FS_ERROR_EXISTS = -4,           // 文件已存在
    EXT2FS_ERROR_PERMISSION = -5,       // 权限不足
    EXT2FS_ERROR_NOT_DIR = -6,          // 不是目录
    EXT2FS_ERROR_IS_DIR = -7,           // 是目录
    EXT2FS_ERROR_NOT_EMPTY = -8,        // 目录不为空
    EXT2FS_ERROR_IO = -9,               // I/O错误
    EXT2FS_ERROR_INVALID = -10,         // 无效参数
} ext2fs_error_t;

// ============================================================================
// 全局变量声明
// ============================================================================
extern FileSystem g_fs;                // 全局文件系统实例

// ============================================================================
// 模块接口声明
// ============================================================================

// 超级块管理 (src/fs/superblock.h)
int superblock_init(void);
int superblock_load(void);
int superblock_save(void);
void superblock_print_info(void);

// inode管理 (src/fs/inode.h)
int inode_alloc(void);
void inode_free(int inode_id);
int inode_read(int inode_id, Inode *inode);
int inode_write(int inode_id, const Inode *inode);
bool inode_is_used(int inode_id);

// 数据块管理 (src/fs/block.h)
int block_alloc(void);
void block_free(int block_id);
int block_read(int block_id, void *buffer);
int block_write(int block_id, const void *buffer);
bool block_is_used(int block_id);

// 目录操作 (src/fs/directory.h)
int dir_create(const char *name, int parent_inode);
int dir_delete(int inode_id);
int dir_find_entry(int dir_inode, const char *name);
int dir_add_entry(int dir_inode, const char *name, int inode_id);
int dir_remove_entry(int dir_inode, const char *name);
int dir_list_entries(int dir_inode, void (*callback)(const char *name, int inode));

// 文件操作 (src/fs/file.h)
int file_create(const char *name, int parent_inode);
int file_delete(int inode_id);
int file_read(int inode_id, void *buffer, size_t size, off_t offset);
int file_write(int inode_id, const void *buffer, size_t size, off_t offset);
int file_truncate(int inode_id, off_t size);
int file_rename(int inode_id, const char *new_name);

// 磁盘I/O (src/core/disk.h)
int disk_init(const char *image_path);
int disk_read(off_t offset, void *buffer, size_t size);
int disk_write(off_t offset, const void *buffer, size_t size);
int disk_sync(void);
void disk_cleanup(void);

// 位图管理 (src/core/bitmap.h)
int bitmap_init(void);
int bitmap_set_bit(char *bitmap, int bit);
int bitmap_clear_bit(char *bitmap, int bit);
bool bitmap_test_bit(const char *bitmap, int bit);
int bitmap_find_free_bit(const char *bitmap, int max_bits);

// FUSE操作接口 (src/fuse/operations.h)
extern struct fuse_operations fuse_operations;

// ============================================================================
// 工具函数
// ============================================================================
mode_t perm_to_mode(const char *perms, bool is_dir);
void mode_to_perm(mode_t mode, char *perms);
const char *get_username(int uid);
int resolve_path(const char *path);
ext2fs_error_t errno_to_ext2fs_error(int errno_val);

#endif /* EXT2FS_H */
