/*
 * ============================================================================
 * 文件名: src/fs/block.h
 * 描述: 数据块管理模块头文件
 * 功能: 数据块的分配、释放、读写和管理
 * ============================================================================
 */

#ifndef BLOCK_H
#define BLOCK_H

#include "../../include/ext2fs.h"

// ============================================================================
// 数据块管理函数
// ============================================================================

/**
 * 初始化数据块管理
 * @return 成功返回0，失败返回负数
 */
int block_init(void);

/**
 * 分配一个空闲数据块
 * @return 成功返回块编号，失败返回负数
 */
int block_alloc(void);

/**
 * 释放一个数据块
 * @param block_id 块编号
 */
void block_free(int block_id);

/**
 * 读取数据块内容
 * @param block_id 块编号
 * @param buffer 缓冲区
 * @return 成功返回读取字节数，失败返回负数
 */
int block_read(int block_id, void *buffer);

/**
 * 写入数据块内容
 * @param block_id 块编号
 * @param buffer 缓冲区
 * @return 成功返回写入字节数，失败返回负数
 */
int block_write(int block_id, const void *buffer);

/**
 * 检查数据块是否被使用
 * @param block_id 块编号
 * @return 被使用返回true，否则返回false
 */
bool block_is_used(int block_id);

/**
 * 清空数据块内容
 * @param block_id 块编号
 * @return 成功返回0，失败返回负数
 */
int block_clear(int block_id);

/**
 * 为inode分配数据块
 * @param inode_id inode编号
 * @return 成功返回新分配的块编号，失败返回负数
 */
int block_alloc_for_inode(int inode_id);

/**
 * 释放inode的所有数据块
 * @param inode_id inode编号
 * @return 成功返回0，失败返回负数
 */
int block_free_all_for_inode(int inode_id);

/**
 * 获取inode的第n个数据块
 * @param inode_id inode编号
 * @param block_index 块索引 (0-based)
 * @return 成功返回块编号，失败返回负数
 */
int block_get_for_inode(int inode_id, int block_index);

/**
 * 计算需要的数据块数量
 * @param size 文件大小
 * @return 需要的块数
 */
int block_count_needed(size_t size);

/**
 * 打印数据块使用统计
 */
void block_print_usage(void);

/**
 * 打印数据块信息 (调试用)
 * @param block_id 块编号
 */
void block_print_info(int block_id);

#endif /* BLOCK_H */
