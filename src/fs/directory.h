/*
 * ============================================================================
 * 文件名: src/fs/directory.h
 * 描述: 目录操作模块头文件
 * 功能: 目录的创建、删除、查找和管理
 * ============================================================================
 */

#ifndef DIRECTORY_H
#define DIRECTORY_H

#include "../../include/ext2fs.h"

// ============================================================================
// 目录操作函数
// ============================================================================

/**
 * 创建目录
 * @param name 目录名
 * @param parent_inode 父目录inode编号
 * @return 成功返回新目录的inode编号，失败返回负数
 */
int dir_create(const char *name, int parent_inode);

/**
 * 删除目录
 * @param inode_id 目录inode编号
 * @return 成功返回0，失败返回负数
 */
int dir_delete(int inode_id);

/**
 * 在目录中查找条目
 * @param dir_inode 目录inode编号
 * @param name 要查找的文件名
 * @return 成功返回找到的inode编号，失败返回负数
 */
int dir_find_entry(int dir_inode, const char *name);

/**
 * 向目录添加条目
 * @param dir_inode 目录inode编号
 * @param name 文件名
 * @param inode_id 文件inode编号
 * @return 成功返回0，失败返回负数
 */
int dir_add_entry(int dir_inode, const char *name, int inode_id);

/**
 * 从目录删除条目
 * @param dir_inode 目录inode编号
 * @param name 文件名
 * @return 成功返回0，失败返回负数
 */
int dir_remove_entry(int dir_inode, const char *name);

/**
 * 列出目录中的所有条目
 * @param dir_inode 目录inode编号
 * @param callback 回调函数，用于处理每个条目
 * @return 成功返回条目数量，失败返回负数
 */
int dir_list_entries(int dir_inode, void (*callback)(const char *name, int inode));

/**
 * 检查目录是否为空
 * @param dir_inode 目录inode编号
 * @return 为空返回true，否则返回false
 */
bool dir_is_empty(int dir_inode);

/**
 * 解析路径，返回对应的inode编号
 * @param path 路径字符串
 * @return 成功返回inode编号，失败返回负数
 */
int dir_resolve_path(const char *path);

/**
 * 解析父目录路径和文件名
 * @param path 完整路径
 * @param parent_inode 输出父目录inode编号
 * @param filename 输出文件名
 * @return 成功返回0，失败返回负数
 */
int dir_parse_path(const char *path, int *parent_inode, char *filename);

/**
 * 获取目录的完整路径
 * @param inode_id 目录inode编号
 * @param path 输出路径缓冲区
 * @param max_len 缓冲区最大长度
 * @return 成功返回0，失败返回负数
 */
int dir_get_full_path(int inode_id, char *path, size_t max_len);

/**
 * 统计目录中的条目数量
 * @param dir_inode 目录inode编号
 * @return 条目数量
 */
int dir_count_entries(int dir_inode);

/**
 * 打印目录信息 (调试用)
 * @param dir_inode 目录inode编号
 */
void dir_print_info(int dir_inode);

#endif /* DIRECTORY_H */
