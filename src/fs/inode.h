/*
 * ============================================================================
 * 文件名: src/fs/inode.h
 * 描述: inode管理模块头文件
 * 功能: inode的分配、释放、读写和管理
 * ============================================================================
 */

#ifndef INODE_H
#define INODE_H

#include "../../include/ext2fs.h"

// ============================================================================
// inode管理函数
// ============================================================================

/**
 * 初始化inode表
 * @return 成功返回0，失败返回负数
 */
int inode_init(void);

/**
 * 加载inode表从磁盘
 * @return 成功返回0，失败返回负数
 */
int inode_load(void);

/**
 * 保存inode表到磁盘
 * @return 成功返回0，失败返回负数
 */
int inode_save(void);

/**
 * 分配一个空闲inode
 * @return 成功返回inode编号，失败返回负数
 */
int inode_alloc(void);

/**
 * 释放一个inode
 * @param inode_id inode编号
 */
void inode_free(int inode_id);

/**
 * 读取inode信息
 * @param inode_id inode编号
 * @param inode 输出的inode结构
 * @return 成功返回0，失败返回负数
 */
int inode_read(int inode_id, Inode *inode);

/**
 * 写入inode信息
 * @param inode_id inode编号
 * @param inode 要写入的inode结构
 * @return 成功返回0，失败返回负数
 */
int inode_write(int inode_id, const Inode *inode);

/**
 * 检查inode是否被使用
 * @param inode_id inode编号
 * @return 被使用返回true，否则返回false
 */
bool inode_is_used(int inode_id);

/**
 * 创建新的inode
 * @param name 文件名
 * @param is_directory 是否为目录
 * @param parent_inode 父目录inode
 * @return 成功返回新inode编号，失败返回负数
 */
int inode_create(const char *name, bool is_directory, int parent_inode);

/**
 * 更新inode的时间戳
 * @param inode_id inode编号
 * @param access 是否更新访问时间
 * @param modify 是否更新修改时间
 * @return 成功返回0，失败返回负数
 */
int inode_update_times(int inode_id, bool access, bool modify);

/**
 * 获取inode的stat信息
 * @param inode_id inode编号
 * @param stbuf 输出的stat结构
 * @return 成功返回0，失败返回负数
 */
int inode_get_stat(int inode_id, struct stat *stbuf);

/**
 * 修改inode权限
 * @param inode_id inode编号
 * @param permissions 新权限字符串
 * @return 成功返回0，失败返回负数
 */
int inode_set_permissions(int inode_id, const char *permissions);

/**
 * 重命名inode
 * @param inode_id inode编号
 * @param new_name 新名称
 * @return 成功返回0，失败返回负数
 */
int inode_rename(int inode_id, const char *new_name);

/**
 * 打印inode信息 (调试用)
 * @param inode_id inode编号
 */
void inode_print_info(int inode_id);

/**
 * 统计inode使用情况
 */
void inode_print_usage(void);

#endif /* INODE_H */
