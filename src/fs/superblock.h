/*
 * ============================================================================
 * 文件名: src/fs/superblock.h
 * 描述: 超级块管理模块头文件
 * 功能: 超级块的创建、加载、保存和管理
 * ============================================================================
 */

#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

#include "../../include/ext2fs.h"

// ============================================================================
// 超级块相关常量
// ============================================================================
#define EXT2FS_MAGIC 0x53EF             // 文件系统魔数
#define SUPERBLOCK_OFFSET 0             // 超级块在磁盘中的偏移量

// ============================================================================
// 超级块管理函数
// ============================================================================

/**
 * 初始化超级块 (格式化时使用)
 * @return 成功返回0，失败返回负数
 */
int superblock_init(void);

/**
 * 从磁盘加载超级块
 * @return 成功返回0，失败返回负数
 */
int superblock_load(void);

/**
 * 将超级块保存到磁盘
 * @return 成功返回0，失败返回负数
 */
int superblock_save(void);

/**
 * 验证超级块的有效性
 * @return 有效返回true，无效返回false
 */
bool superblock_is_valid(void);

/**
 * 更新超级块的统计信息
 */
void superblock_update_stats(void);

/**
 * 打印超级块信息 (调试用)
 */
void superblock_print_info(void);

/**
 * 获取超级块指针
 * @return 超级块指针
 */
SuperBlock* superblock_get(void);

#endif /* SUPERBLOCK_H */
