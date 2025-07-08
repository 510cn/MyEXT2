/*
 * ============================================================================
 * 文件名: src/fs/directory.c
 * 描述: 目录操作模块实现
 * 功能: 目录的创建、删除、查找和管理
 * ============================================================================
 */

#include "directory.h"
#include "inode.h"
#include "block.h"

// ============================================================================
// 目录操作函数实现
// ============================================================================

/**
 * 创建目录
 */
int dir_create(const char *name, int parent_inode) {
    if (!name || strlen(name) >= MAX_FILENAME) {
        return -1;
    }
    
    // 检查父目录是否存在且为目录
    if (!inode_is_used(parent_inode) || !g_fs.inode_table[parent_inode].is_directory) {
        return -1;
    }
    
    // 检查是否已存在同名文件
    if (dir_find_entry(parent_inode, name) != -1) {
        return -1;  // 已存在
    }
    
    // 创建新的inode
    int new_inode = inode_create(name, true, parent_inode);
    if (new_inode == -1) {
        return -1;
    }
    
    // 为新目录分配数据块
    int block_id = block_alloc_for_inode(new_inode);
    if (block_id == -1) {
        inode_free(new_inode);
        return -1;
    }
    
    return new_inode;
}

/**
 * 删除目录
 */
int dir_delete(int inode_id) {
    if (!inode_is_used(inode_id) || !g_fs.inode_table[inode_id].is_directory) {
        return -1;
    }
    
    // 检查目录是否为空
    if (!dir_is_empty(inode_id)) {
        return -1;  // 目录不为空
    }
    
    // 释放所有数据块
    block_free_all_for_inode(inode_id);
    
    // 释放inode
    inode_free(inode_id);
    
    return 0;
}

/**
 * 在目录中查找条目
 */
int dir_find_entry(int dir_inode, const char *name) {
    if (!inode_is_used(dir_inode) || !g_fs.inode_table[dir_inode].is_directory || !name) {
        return -1;
    }
    
    // 简化实现：遍历所有inode查找匹配的子项
    for (int i = 0; i < MAX_INODES; i++) {
        if (inode_is_used(i) && 
            g_fs.inode_table[i].parent_inode == dir_inode &&
            strcmp(g_fs.inode_table[i].name, name) == 0) {
            return i;
        }
    }
    
    return -1;  // 未找到
}

/**
 * 向目录添加条目
 */
int dir_add_entry(int dir_inode, const char *name, int inode_id) {
    if (!inode_is_used(dir_inode) || !g_fs.inode_table[dir_inode].is_directory ||
        !name || !inode_is_used(inode_id)) {
        return -1;
    }
    
    // 检查是否已存在同名条目
    if (dir_find_entry(dir_inode, name) != -1) {
        return -1;  // 已存在
    }
    
    // 更新目标inode的父目录和名称
    g_fs.inode_table[inode_id].parent_inode = dir_inode;
    strncpy(g_fs.inode_table[inode_id].name, name, MAX_FILENAME - 1);
    g_fs.inode_table[inode_id].name[MAX_FILENAME - 1] = '\0';
    
    // 更新目录的修改时间
    inode_update_times(dir_inode, false, true);
    
    g_fs.is_dirty = true;
    return 0;
}

/**
 * 从目录删除条目
 */
int dir_remove_entry(int dir_inode, const char *name) {
    if (!inode_is_used(dir_inode) || !g_fs.inode_table[dir_inode].is_directory || !name) {
        return -1;
    }
    
    // 查找条目
    int target_inode = dir_find_entry(dir_inode, name);
    if (target_inode == -1) {
        return -1;  // 未找到
    }
    
    // 如果是目录，检查是否为空
    if (g_fs.inode_table[target_inode].is_directory && !dir_is_empty(target_inode)) {
        return -1;  // 目录不为空
    }
    
    // 释放目标inode的资源
    if (g_fs.inode_table[target_inode].is_directory) {
        dir_delete(target_inode);
    } else {
        block_free_all_for_inode(target_inode);
        inode_free(target_inode);
    }
    
    // 更新目录的修改时间
    inode_update_times(dir_inode, false, true);
    
    return 0;
}

/**
 * 列出目录中的所有条目
 */
int dir_list_entries(int dir_inode, void (*callback)(const char *name, int inode)) {
    if (!inode_is_used(dir_inode) || !g_fs.inode_table[dir_inode].is_directory || !callback) {
        return -1;
    }
    
    int count = 0;
    
    // 遍历所有inode查找子项
    for (int i = 0; i < MAX_INODES; i++) {
        if (inode_is_used(i) && g_fs.inode_table[i].parent_inode == dir_inode) {
            callback(g_fs.inode_table[i].name, i);
            count++;
        }
    }
    
    // 更新目录的访问时间
    inode_update_times(dir_inode, true, false);
    
    return count;
}

/**
 * 检查目录是否为空
 */
bool dir_is_empty(int dir_inode) {
    if (!inode_is_used(dir_inode) || !g_fs.inode_table[dir_inode].is_directory) {
        return false;
    }
    
    // 检查是否有子项
    for (int i = 0; i < MAX_INODES; i++) {
        if (inode_is_used(i) && g_fs.inode_table[i].parent_inode == dir_inode) {
            return false;  // 找到子项，不为空
        }
    }
    
    return true;  // 为空
}

/**
 * 解析路径，返回对应的inode编号
 */
int dir_resolve_path(const char *path) {
    if (!path) {
        return -1;
    }
    
    // 根目录
    if (strcmp(path, "/") == 0) {
        return ROOT_INODE;
    }
    
    // 复制路径以便修改
    char *path_copy = strdup(path);
    if (!path_copy) {
        return -1;
    }
    
    int current_inode = ROOT_INODE;
    char *token = strtok(path_copy + 1, "/");  // 跳过开头的'/'
    
    while (token != NULL) {
        int found = dir_find_entry(current_inode, token);
        if (found == -1) {
            free(path_copy);
            return -1;  // 路径不存在
        }
        current_inode = found;
        token = strtok(NULL, "/");
    }
    
    free(path_copy);
    return current_inode;
}

/**
 * 解析父目录路径和文件名
 */
int dir_parse_path(const char *path, int *parent_inode, char *filename) {
    if (!path || !parent_inode || !filename) {
        return -1;
    }
    
    // 复制路径
    char *path_copy = strdup(path);
    if (!path_copy) {
        return -1;
    }
    
    // 查找最后一个'/'
    char *last_slash = strrchr(path_copy, '/');
    if (!last_slash) {
        free(path_copy);
        return -1;  // 无效路径
    }
    
    // 提取文件名
    strcpy(filename, last_slash + 1);
    
    // 如果是根目录下的文件
    if (last_slash == path_copy) {
        *parent_inode = ROOT_INODE;
    } else {
        // 截断路径获取父目录
        *last_slash = '\0';
        *parent_inode = dir_resolve_path(path_copy);
        if (*parent_inode == -1) {
            free(path_copy);
            return -1;
        }
    }
    
    free(path_copy);
    return 0;
}

/**
 * 获取目录的完整路径
 */
int dir_get_full_path(int inode_id, char *path, size_t max_len) {
    if (!inode_is_used(inode_id) || !path || max_len == 0) {
        return -1;
    }
    
    // 根目录特殊处理
    if (inode_id == ROOT_INODE) {
        strncpy(path, "/", max_len - 1);
        path[max_len - 1] = '\0';
        return 0;
    }
    
    // 递归构建路径
    char temp_path[1024] = "";
    int current = inode_id;
    
    while (current != ROOT_INODE && current != -1) {
        Inode *inode = &g_fs.inode_table[current];
        
        // 在路径前面添加当前目录名
        char new_temp[1024];
        snprintf(new_temp, sizeof(new_temp), "/%s%s", inode->name, temp_path);
        strcpy(temp_path, new_temp);
        
        current = inode->parent_inode;
    }
    
    // 复制到输出缓冲区
    strncpy(path, temp_path, max_len - 1);
    path[max_len - 1] = '\0';
    
    return 0;
}

/**
 * 统计目录中的条目数量
 */
int dir_count_entries(int dir_inode) {
    if (!inode_is_used(dir_inode) || !g_fs.inode_table[dir_inode].is_directory) {
        return -1;
    }
    
    int count = 0;
    
    for (int i = 0; i < MAX_INODES; i++) {
        if (inode_is_used(i) && g_fs.inode_table[i].parent_inode == dir_inode) {
            count++;
        }
    }
    
    return count;
}

/**
 * 打印目录信息 (调试用)
 */
void dir_print_info(int dir_inode) {
    if (!inode_is_used(dir_inode) || !g_fs.inode_table[dir_inode].is_directory) {
        printf("目录 %d: 无效或不是目录\n", dir_inode);
        return;
    }
    
    Inode *inode = &g_fs.inode_table[dir_inode];
    char full_path[1024];
    dir_get_full_path(dir_inode, full_path, sizeof(full_path));
    
    printf("\n=== 目录 %d 信息 ===\n", dir_inode);
    printf("名称: %s\n", inode->name);
    printf("完整路径: %s\n", full_path);
    printf("父目录: %d\n", inode->parent_inode);
    printf("条目数量: %d\n", dir_count_entries(dir_inode));
    printf("是否为空: %s\n", dir_is_empty(dir_inode) ? "是" : "否");
    printf("创建时间: %s", ctime(&inode->created));
    printf("修改时间: %s", ctime(&inode->modified));
    
    printf("子条目:\n");
    for (int i = 0; i < MAX_INODES; i++) {
        if (inode_is_used(i) && g_fs.inode_table[i].parent_inode == dir_inode) {
            printf("  %s (%s)\n", g_fs.inode_table[i].name,
                   g_fs.inode_table[i].is_directory ? "目录" : "文件");
        }
    }
    
    printf("==================\n\n");
}
