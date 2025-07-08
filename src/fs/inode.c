/*
 * ============================================================================
 * 文件名: src/fs/inode.c
 * 描述: inode管理模块实现
 * 功能: inode的分配、释放、读写和管理
 * ============================================================================
 */

#include "inode.h"
#include "../core/disk.h"
#include "../core/bitmap.h"

// ============================================================================
// inode管理函数实现
// ============================================================================

/**
 * 初始化inode表
 */
int inode_init(void) {
    // 分配inode表内存
    g_fs.inode_table = calloc(MAX_INODES, sizeof(Inode));
    if (!g_fs.inode_table) {
        printf("错误: 无法分配inode表内存\n");
        return -1;
    }
    
    // 创建根目录inode
    Inode *root = &g_fs.inode_table[ROOT_INODE];
    strcpy(root->name, "/");
    root->owner_id = 0;
    strcpy(root->permissions, "rwx");
    root->size = 0;
    root->data_blocks[0] = 0;  // 根目录使用第0个数据块
    root->block_count = 1;
    
    time_t now = time(NULL);
    root->created = now;
    root->modified = now;
    root->accessed = now;
    root->is_directory = 1;
    root->parent_inode = ROOT_INODE;  // 根目录的父目录是自己
    root->link_count = 1;
    
    printf("inode表初始化完成\n");
    return 0;
}

/**
 * 加载inode表从磁盘
 */
int inode_load(void) {
    SuperBlock *sb = &g_fs.superblock;
    
    // 分配内存
    g_fs.inode_table = malloc(MAX_INODES * sizeof(Inode));
    if (!g_fs.inode_table) {
        printf("错误: 无法分配inode表内存\n");
        return -1;
    }
    
    // 从磁盘读取inode表
    if (disk_read(sb->inode_table_offset, g_fs.inode_table, 
                  MAX_INODES * sizeof(Inode)) < 0) {
        printf("错误: 无法读取inode表\n");
        free(g_fs.inode_table);
        return -1;
    }
    
    printf("inode表加载完成\n");
    return 0;
}

/**
 * 保存inode表到磁盘
 */
int inode_save(void) {
    SuperBlock *sb = &g_fs.superblock;
    
    if (!g_fs.inode_table) {
        printf("错误: inode表未初始化\n");
        return -1;
    }
    
    // 写入磁盘
    if (disk_write(sb->inode_table_offset, g_fs.inode_table, 
                   MAX_INODES * sizeof(Inode)) < 0) {
        printf("错误: 无法写入inode表\n");
        return -1;
    }
    
    return 0;
}

/**
 * 分配一个空闲inode
 */
int inode_alloc(void) {
    int inode_id = bitmap_find_free_bit(g_fs.inode_bitmap, MAX_INODES);
    if (inode_id == -1) {
        printf("错误: 没有空闲inode\n");
        return -1;
    }
    
    // 标记为已使用
    bitmap_set_bit(g_fs.inode_bitmap, inode_id);
    
    // 清空inode内容
    memset(&g_fs.inode_table[inode_id], 0, sizeof(Inode));
    
    // 更新超级块统计
    g_fs.superblock.free_inodes--;
    g_fs.is_dirty = true;
    
    return inode_id;
}

/**
 * 释放一个inode
 */
void inode_free(int inode_id) {
    if (inode_id < 0 || inode_id >= MAX_INODES) {
        return;
    }
    
    // 清除位图标记
    bitmap_clear_bit(g_fs.inode_bitmap, inode_id);
    
    // 清空inode内容
    memset(&g_fs.inode_table[inode_id], 0, sizeof(Inode));
    
    // 更新超级块统计
    g_fs.superblock.free_inodes++;
    g_fs.is_dirty = true;
}

/**
 * 读取inode信息
 */
int inode_read(int inode_id, Inode *inode) {
    if (inode_id < 0 || inode_id >= MAX_INODES || !inode) {
        return -1;
    }
    
    if (!inode_is_used(inode_id)) {
        return -1;
    }
    
    *inode = g_fs.inode_table[inode_id];
    return 0;
}

/**
 * 写入inode信息
 */
int inode_write(int inode_id, const Inode *inode) {
    if (inode_id < 0 || inode_id >= MAX_INODES || !inode) {
        return -1;
    }
    
    g_fs.inode_table[inode_id] = *inode;
    g_fs.is_dirty = true;
    
    return 0;
}

/**
 * 检查inode是否被使用
 */
bool inode_is_used(int inode_id) {
    if (inode_id < 0 || inode_id >= MAX_INODES) {
        return false;
    }
    
    return bitmap_test_bit(g_fs.inode_bitmap, inode_id);
}

/**
 * 创建新的inode
 */
int inode_create(const char *name, bool is_directory, int parent_inode) {
    if (!name || strlen(name) >= MAX_FILENAME) {
        return -1;
    }
    
    // 分配新inode
    int inode_id = inode_alloc();
    if (inode_id == -1) {
        return -1;
    }
    
    // 初始化inode
    Inode *inode = &g_fs.inode_table[inode_id];
    strcpy(inode->name, name);
    inode->owner_id = 0;  // 默认root用户
    strcpy(inode->permissions, "rwx");
    inode->size = 0;
    inode->block_count = 0;
    
    time_t now = time(NULL);
    inode->created = now;
    inode->modified = now;
    inode->accessed = now;
    inode->is_directory = is_directory ? 1 : 0;
    inode->parent_inode = parent_inode;
    inode->link_count = 1;
    
    // 清空数据块索引
    for (int i = 0; i < MAX_DIRECT_BLOCKS; i++) {
        inode->data_blocks[i] = 0;
    }
    
    g_fs.is_dirty = true;
    return inode_id;
}

/**
 * 更新inode的时间戳
 */
int inode_update_times(int inode_id, bool access, bool modify) {
    if (!inode_is_used(inode_id)) {
        return -1;
    }
    
    Inode *inode = &g_fs.inode_table[inode_id];
    time_t now = time(NULL);
    
    if (access) {
        inode->accessed = now;
    }
    if (modify) {
        inode->modified = now;
    }
    
    g_fs.is_dirty = true;
    return 0;
}

/**
 * 获取inode的stat信息
 */
int inode_get_stat(int inode_id, struct stat *stbuf) {
    if (!inode_is_used(inode_id) || !stbuf) {
        return -1;
    }
    
    Inode *inode = &g_fs.inode_table[inode_id];
    
    memset(stbuf, 0, sizeof(struct stat));
    
    // 设置文件类型和权限
    if (inode->is_directory) {
        stbuf->st_mode = S_IFDIR | 0755;
    } else {
        stbuf->st_mode = S_IFREG | 0644;
    }
    
    // 根据权限字符串设置权限位
    if (strchr(inode->permissions, 'r')) {
        stbuf->st_mode |= S_IRUSR | S_IRGRP | S_IROTH;
    }
    if (strchr(inode->permissions, 'w')) {
        stbuf->st_mode |= S_IWUSR | S_IWGRP | S_IWOTH;
    }
    if (strchr(inode->permissions, 'x')) {
        stbuf->st_mode |= S_IXUSR | S_IXGRP | S_IXOTH;
    }
    
    stbuf->st_nlink = inode->link_count;
    stbuf->st_uid = inode->owner_id;
    stbuf->st_gid = 0;
    stbuf->st_size = inode->size;
    stbuf->st_blocks = inode->block_count;
    stbuf->st_blksize = BLOCK_SIZE;
    
    stbuf->st_atime = inode->accessed;
    stbuf->st_mtime = inode->modified;
    stbuf->st_ctime = inode->created;
    
    return 0;
}

/**
 * 修改inode权限
 */
int inode_set_permissions(int inode_id, const char *permissions) {
    if (!inode_is_used(inode_id) || !permissions) {
        return -1;
    }
    
    Inode *inode = &g_fs.inode_table[inode_id];
    strncpy(inode->permissions, permissions, sizeof(inode->permissions) - 1);
    inode->permissions[sizeof(inode->permissions) - 1] = '\0';
    
    inode_update_times(inode_id, false, true);
    return 0;
}

/**
 * 重命名inode
 */
int inode_rename(int inode_id, const char *new_name) {
    if (!inode_is_used(inode_id) || !new_name || strlen(new_name) >= MAX_FILENAME) {
        return -1;
    }
    
    Inode *inode = &g_fs.inode_table[inode_id];
    strcpy(inode->name, new_name);
    
    inode_update_times(inode_id, false, true);
    return 0;
}

/**
 * 打印inode信息 (调试用)
 */
void inode_print_info(int inode_id) {
    if (!inode_is_used(inode_id)) {
        printf("inode %d: 未使用\n", inode_id);
        return;
    }
    
    Inode *inode = &g_fs.inode_table[inode_id];
    
    printf("\n=== inode %d 信息 ===\n", inode_id);
    printf("名称: %s\n", inode->name);
    printf("类型: %s\n", inode->is_directory ? "目录" : "文件");
    printf("所有者: %u\n", inode->owner_id);
    printf("权限: %s\n", inode->permissions);
    printf("大小: %u 字节\n", inode->size);
    printf("数据块数: %u\n", inode->block_count);
    printf("硬链接数: %u\n", inode->link_count);
    printf("父目录: %u\n", inode->parent_inode);
    printf("创建时间: %s", ctime(&inode->created));
    printf("修改时间: %s", ctime(&inode->modified));
    printf("访问时间: %s", ctime(&inode->accessed));
    printf("数据块: ");
    for (int i = 0; i < inode->block_count && i < MAX_DIRECT_BLOCKS; i++) {
        printf("%u ", inode->data_blocks[i]);
    }
    printf("\n==================\n\n");
}

/**
 * 统计inode使用情况
 */
void inode_print_usage(void) {
    int used = bitmap_count_used_bits(g_fs.inode_bitmap, MAX_INODES);
    int free = MAX_INODES - used;
    
    printf("\n=== inode使用统计 ===\n");
    printf("总inode数: %d\n", MAX_INODES);
    printf("已使用: %d\n", used);
    printf("空闲: %d\n", free);
    printf("使用率: %.1f%%\n", (float)used / MAX_INODES * 100);
    printf("==================\n\n");
}
