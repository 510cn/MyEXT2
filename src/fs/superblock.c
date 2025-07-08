/*
 * ============================================================================
 * 文件名: src/fs/superblock.c
 * 描述: 超级块管理模块实现
 * 功能: 超级块的创建、加载、保存和管理
 * ============================================================================
 */

#include "superblock.h"
#include "../core/disk.h"

// ============================================================================
// 超级块管理函数实现
// ============================================================================

/**
 * 初始化超级块 (格式化时使用)
 */
int superblock_init(void) {
    SuperBlock *sb = &g_fs.superblock;
    
    // 设置魔数
    sb->magic = EXT2FS_MAGIC;
    
    // 设置基本参数
    sb->total_blocks = MAX_BLOCKS;
    sb->free_blocks = MAX_BLOCKS - 1;       // 减1是因为根目录占用一个块
    sb->total_inodes = MAX_INODES;
    sb->free_inodes = MAX_INODES - 1;       // 减1是因为根目录占用一个inode
    sb->block_size = BLOCK_SIZE;
    sb->inode_size = INODE_SIZE;
    
    // 计算各区域在磁盘中的偏移量
    sb->inode_bitmap_offset = sizeof(SuperBlock);
    sb->block_bitmap_offset = sb->inode_bitmap_offset + (MAX_INODES / 8);
    sb->inode_table_offset = sb->block_bitmap_offset + (MAX_BLOCKS / 8);
    sb->data_blocks_offset = sb->inode_table_offset + (MAX_INODES * sizeof(Inode));
    
    // 设置时间戳
    time_t now = time(NULL);
    sb->created = now;
    sb->last_mount = now;
    sb->mount_count = 1;
    
    printf("超级块初始化完成\n");
    printf("- 总块数: %u\n", sb->total_blocks);
    printf("- 总inode数: %u\n", sb->total_inodes);
    printf("- 块大小: %u 字节\n", sb->block_size);
    
    return 0;
}

/**
 * 从磁盘加载超级块
 */
int superblock_load(void) {
    SuperBlock *sb = &g_fs.superblock;
    
    // 从磁盘读取超级块
    if (disk_read(SUPERBLOCK_OFFSET, sb, sizeof(SuperBlock)) != 0) {
        printf("错误: 无法从磁盘读取超级块\n");
        return -1;
    }
    
    // 验证魔数
    if (sb->magic != EXT2FS_MAGIC) {
        printf("错误: 无效的文件系统魔数 (期望: 0x%X, 实际: 0x%X)\n", 
               EXT2FS_MAGIC, sb->magic);
        return -1;
    }
    
    // 更新挂载信息
    sb->last_mount = time(NULL);
    sb->mount_count++;
    
    printf("超级块加载完成\n");
    printf("- 文件系统版本: %s\n", EXT2FS_VERSION);
    printf("- 创建时间: %s", ctime(&sb->created));
    printf("- 挂载次数: %u\n", sb->mount_count);
    
    return 0;
}

/**
 * 将超级块保存到磁盘
 */
int superblock_save(void) {
    SuperBlock *sb = &g_fs.superblock;
    
    // 写入磁盘
    if (disk_write(SUPERBLOCK_OFFSET, sb, sizeof(SuperBlock)) != 0) {
        printf("错误: 无法将超级块写入磁盘\n");
        return -1;
    }
    
    return 0;
}

/**
 * 验证超级块的有效性
 */
bool superblock_is_valid(void) {
    SuperBlock *sb = &g_fs.superblock;
    
    // 检查魔数
    if (sb->magic != EXT2FS_MAGIC) {
        return false;
    }
    
    // 检查基本参数的合理性
    if (sb->total_blocks == 0 || sb->total_inodes == 0) {
        return false;
    }
    
    if (sb->free_blocks > sb->total_blocks || sb->free_inodes > sb->total_inodes) {
        return false;
    }
    
    if (sb->block_size != BLOCK_SIZE || sb->inode_size != INODE_SIZE) {
        return false;
    }
    
    return true;
}

/**
 * 更新超级块的统计信息
 */
void superblock_update_stats(void) {
    SuperBlock *sb = &g_fs.superblock;
    
    // 重新计算空闲inode和块的数量
    uint32_t free_inodes = 0;
    uint32_t free_blocks = 0;
    
    // 统计空闲inode
    for (int i = 0; i < MAX_INODES; i++) {
        if (!bitmap_test_bit(g_fs.inode_bitmap, i)) {
            free_inodes++;
        }
    }
    
    // 统计空闲块
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (!bitmap_test_bit(g_fs.block_bitmap, i)) {
            free_blocks++;
        }
    }
    
    sb->free_inodes = free_inodes;
    sb->free_blocks = free_blocks;
    
    // 标记文件系统为脏
    g_fs.is_dirty = true;
}

/**
 * 打印超级块信息 (调试用)
 */
void superblock_print_info(void) {
    SuperBlock *sb = &g_fs.superblock;
    
    printf("\n=== 超级块信息 ===\n");
    printf("魔数: 0x%X\n", sb->magic);
    printf("总块数: %u\n", sb->total_blocks);
    printf("空闲块数: %u\n", sb->free_blocks);
    printf("总inode数: %u\n", sb->total_inodes);
    printf("空闲inode数: %u\n", sb->free_inodes);
    printf("块大小: %u 字节\n", sb->block_size);
    printf("inode大小: %u 字节\n", sb->inode_size);
    printf("创建时间: %s", ctime(&sb->created));
    printf("最后挂载: %s", ctime(&sb->last_mount));
    printf("挂载次数: %u\n", sb->mount_count);
    printf("==================\n\n");
}

/**
 * 获取超级块指针
 */
SuperBlock* superblock_get(void) {
    return &g_fs.superblock;
}
