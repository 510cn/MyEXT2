/*
 * ============================================================================
 * 文件名: src/fuse/operations.h
 * 描述: FUSE操作接口头文件
 * 功能: 定义FUSE操作函数和相关结构
 * ============================================================================
 */

#ifndef FUSE_OPERATIONS_H
#define FUSE_OPERATIONS_H

#define FUSE_USE_VERSION 30
#define _GNU_SOURCE

#include <fuse.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include "../../include/ext2fs.h"

// 确保时间常量定义
#ifndef UTIME_OMIT
#define UTIME_OMIT ((1l << 30) - 2l)
#endif

// ============================================================================
// FUSE操作函数声明
// ============================================================================

/**
 * 获取文件属性
 */
static int fuse_getattr(const char *path, struct stat *stbuf);

/**
 * 读取目录内容
 */
static int fuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                        off_t offset, struct fuse_file_info *fi);

/**
 * 打开文件
 */
static int fuse_open(const char *path, struct fuse_file_info *fi);

/**
 * 读取文件内容
 */
static int fuse_read(const char *path, char *buf, size_t size, off_t offset,
                     struct fuse_file_info *fi);

/**
 * 写入文件内容
 */
static int fuse_write(const char *path, const char *buf, size_t size,
                      off_t offset, struct fuse_file_info *fi);

/**
 * 创建文件
 */
static int fuse_create(const char *path, mode_t mode, struct fuse_file_info *fi);

/**
 * 创建目录
 */
static int fuse_mkdir(const char *path, mode_t mode);

/**
 * 删除文件
 */
static int fuse_unlink(const char *path);

/**
 * 删除目录
 */
static int fuse_rmdir(const char *path);

/**
 * 重命名文件/目录
 */
static int fuse_rename(const char *from, const char *to);

/**
 * 修改文件权限
 */
static int fuse_chmod(const char *path, mode_t mode);

/**
 * 截断文件
 */
static int fuse_truncate(const char *path, off_t size);

/**
 * 更新文件时间
 */
static int fuse_utimens(const char *path, const struct timespec ts[2]);

/**
 * 获取文件系统统计信息
 */
static int fuse_statfs(const char *path, struct statvfs *stbuf);

/**
 * FUSE初始化
 */
static void *fuse_init(struct fuse_conn_info *conn);

/**
 * FUSE清理
 */
static void fuse_cleanup(void *userdata);

/**
 * 打开目录
 */
static int fuse_opendir(const char *path, struct fuse_file_info *fi);

/**
 * 释放文件句柄
 */
static int fuse_release(const char *path, struct fuse_file_info *fi);

/**
 * 释放目录句柄
 */
static int fuse_releasedir(const char *path, struct fuse_file_info *fi);

/**
 * 同步文件数据
 */
static int fuse_fsync(const char *path, int isdatasync, struct fuse_file_info *fi);

// ============================================================================
// 辅助函数声明
// ============================================================================

/**
 * 解析父目录路径和文件名
 */
static int parse_parent_path(const char *path, char *filename);

/**
 * 将POSIX错误码转换为FUSE错误码
 */
static int errno_to_fuse_error(ext2fs_error_t error);

/**
 * 将权限模式转换为权限字符串
 */
static void mode_to_permissions(mode_t mode, char *perms);

/**
 * 将权限字符串转换为权限模式
 */
static mode_t permissions_to_mode(const char *perms, bool is_dir);

// ============================================================================
// 全局FUSE操作结构体
// ============================================================================
extern struct fuse_operations fuse_operations;

#endif /* FUSE_OPERATIONS_H */
