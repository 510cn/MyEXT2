/*
 * ============================================================================
 * 文件名: src/fs/file.h
 * 描述: 文件操作模块头文件
 * 功能: 文件的创建、删除、读写和管理
 * ============================================================================
 */

#ifndef FILE_H
#define FILE_H

#include "../../include/ext2fs.h"

// ============================================================================
// 文件操作函数
// ============================================================================

/**
 * 创建文件
 * @param name 文件名
 * @param parent_inode 父目录inode编号
 * @return 成功返回新文件的inode编号，失败返回负数
 */
int file_create(const char *name, int parent_inode);

/**
 * 删除文件
 * @param inode_id 文件inode编号
 * @return 成功返回0，失败返回负数
 */
int file_delete(int inode_id);

/**
 * 读取文件内容
 * @param inode_id 文件inode编号
 * @param buffer 缓冲区
 * @param size 读取大小
 * @param offset 偏移量
 * @return 成功返回实际读取字节数，失败返回负数
 */
int file_read(int inode_id, void *buffer, size_t size, off_t offset);

/**
 * 写入文件内容
 * @param inode_id 文件inode编号
 * @param buffer 缓冲区
 * @param size 写入大小
 * @param offset 偏移量
 * @return 成功返回实际写入字节数，失败返回负数
 */
int file_write(int inode_id, const void *buffer, size_t size, off_t offset);

/**
 * 截断文件
 * @param inode_id 文件inode编号
 * @param size 新大小
 * @return 成功返回0，失败返回负数
 */
int file_truncate(int inode_id, off_t size);

/**
 * 重命名文件
 * @param inode_id 文件inode编号
 * @param new_name 新名称
 * @return 成功返回0，失败返回负数
 */
int file_rename(int inode_id, const char *new_name);

/**
 * 复制文件
 * @param src_inode 源文件inode编号
 * @param dst_name 目标文件名
 * @param dst_parent 目标父目录inode编号
 * @return 成功返回新文件inode编号，失败返回负数
 */
int file_copy(int src_inode, const char *dst_name, int dst_parent);

/**
 * 移动文件
 * @param inode_id 文件inode编号
 * @param new_parent 新父目录inode编号
 * @param new_name 新名称 (可选，NULL表示保持原名)
 * @return 成功返回0，失败返回负数
 */
int file_move(int inode_id, int new_parent, const char *new_name);

/**
 * 获取文件大小
 * @param inode_id 文件inode编号
 * @return 文件大小，失败返回负数
 */
off_t file_get_size(int inode_id);

/**
 * 设置文件大小
 * @param inode_id 文件inode编号
 * @param size 新大小
 * @return 成功返回0，失败返回负数
 */
int file_set_size(int inode_id, off_t size);

/**
 * 检查文件是否存在
 * @param parent_inode 父目录inode编号
 * @param name 文件名
 * @return 存在返回inode编号，不存在返回负数
 */
int file_exists(int parent_inode, const char *name);

/**
 * 分配文件所需的数据块
 * @param inode_id 文件inode编号
 * @param size 文件大小
 * @return 成功返回0，失败返回负数
 */
int file_alloc_blocks(int inode_id, size_t size);

/**
 * 释放文件多余的数据块
 * @param inode_id 文件inode编号
 * @param new_size 新文件大小
 * @return 成功返回0，失败返回负数
 */
int file_free_excess_blocks(int inode_id, size_t new_size);

/**
 * 打印文件信息 (调试用)
 * @param inode_id 文件inode编号
 */
void file_print_info(int inode_id);

/**
 * 验证文件完整性
 * @param inode_id 文件inode编号
 * @return 完整返回true，否则返回false
 */
bool file_verify_integrity(int inode_id);

#endif /* FILE_H */
