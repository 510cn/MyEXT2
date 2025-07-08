/*
 * ============================================================================
 * 文件名: src/core/disk.h
 * 描述: 磁盘I/O模块头文件
 * 功能: 虚拟磁盘文件的读写操作
 * ============================================================================
 */

#ifndef DISK_H
#define DISK_H

#include "../../include/ext2fs.h"

// ============================================================================
// 磁盘I/O函数
// ============================================================================

/**
 * 初始化磁盘I/O系统
 * @param image_path 磁盘镜像文件路径
 * @return 成功返回0，失败返回负数
 */
int disk_init(const char *image_path);

/**
 * 创建新的磁盘镜像文件
 * @param image_path 磁盘镜像文件路径
 * @return 成功返回0，失败返回负数
 */
int disk_create(const char *image_path);

/**
 * 打开现有的磁盘镜像文件
 * @param image_path 磁盘镜像文件路径
 * @return 成功返回0，失败返回负数
 */
int disk_open(const char *image_path);

/**
 * 从磁盘读取数据
 * @param offset 偏移量
 * @param buffer 缓冲区
 * @param size 读取大小
 * @return 成功返回实际读取字节数，失败返回负数
 */
int disk_read(off_t offset, void *buffer, size_t size);

/**
 * 向磁盘写入数据
 * @param offset 偏移量
 * @param buffer 缓冲区
 * @param size 写入大小
 * @return 成功返回实际写入字节数，失败返回负数
 */
int disk_write(off_t offset, const void *buffer, size_t size);

/**
 * 同步磁盘数据 (强制写入)
 * @return 成功返回0，失败返回负数
 */
int disk_sync(void);

/**
 * 获取磁盘大小
 * @return 磁盘大小 (字节)
 */
off_t disk_get_size(void);

/**
 * 检查磁盘是否已打开
 * @return 已打开返回true，否则返回false
 */
bool disk_is_open(void);

/**
 * 清理磁盘I/O系统
 */
void disk_cleanup(void);

/**
 * 打印磁盘统计信息
 */
void disk_print_stats(void);

#endif /* DISK_H */
