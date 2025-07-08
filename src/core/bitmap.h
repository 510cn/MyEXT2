/*
 * ============================================================================
 * 文件名: src/core/bitmap.h
 * 描述: 位图管理模块头文件
 * 功能: inode和数据块位图的管理
 * ============================================================================
 */

#ifndef BITMAP_H
#define BITMAP_H

#include "../../include/ext2fs.h"

// ============================================================================
// 位图管理函数
// ============================================================================

/**
 * 初始化位图
 * @return 成功返回0，失败返回负数
 */
int bitmap_init(void);

/**
 * 加载位图从磁盘
 * @return 成功返回0，失败返回负数
 */
int bitmap_load(void);

/**
 * 保存位图到磁盘
 * @return 成功返回0，失败返回负数
 */
int bitmap_save(void);

/**
 * 设置位图中的某一位
 * @param bitmap 位图指针
 * @param bit 位编号
 * @return 成功返回0，失败返回负数
 */
int bitmap_set_bit(char *bitmap, int bit);

/**
 * 清除位图中的某一位
 * @param bitmap 位图指针
 * @param bit 位编号
 * @return 成功返回0，失败返回负数
 */
int bitmap_clear_bit(char *bitmap, int bit);

/**
 * 测试位图中的某一位是否被设置
 * @param bitmap 位图指针
 * @param bit 位编号
 * @return 被设置返回true，否则返回false
 */
bool bitmap_test_bit(const char *bitmap, int bit);

/**
 * 在位图中查找第一个空闲位
 * @param bitmap 位图指针
 * @param max_bits 最大位数
 * @return 找到返回位编号，未找到返回-1
 */
int bitmap_find_free_bit(const char *bitmap, int max_bits);

/**
 * 统计位图中已使用的位数
 * @param bitmap 位图指针
 * @param max_bits 最大位数
 * @return 已使用的位数
 */
int bitmap_count_used_bits(const char *bitmap, int max_bits);

/**
 * 统计位图中空闲的位数
 * @param bitmap 位图指针
 * @param max_bits 最大位数
 * @return 空闲的位数
 */
int bitmap_count_free_bits(const char *bitmap, int max_bits);

/**
 * 打印位图状态 (调试用)
 * @param bitmap 位图指针
 * @param max_bits 最大位数
 * @param name 位图名称
 */
void bitmap_print_status(const char *bitmap, int max_bits, const char *name);

#endif /* BITMAP_H */
