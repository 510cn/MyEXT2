/*
 * ============================================================================
 * 文件名: src/fs/block.c
 * 描述: 数据块管理模块实现
 * 功能: 数据块的分配、释放、读写和管理
 * ============================================================================
 */

#include "block.h"
#include "inode.h"
#include "../core/disk.h"
#include "../core/bitmap.h"

// ============================================================================
// 数据块管理函数实现
// ============================================================================

/**
 * 初始化数据块管理
 */
int block_init(void) {
    // 标记第0块为已使用 (用于用户信息存储)
    bitmap_set_bit(g_fs.block_bitmap, 0);
    
    printf("数据块管理初始化完成\n");
    return 0;
}

/**
 * 分配一个空闲数据块
 */
int block_alloc(void) {
    // 从第1块开始查找 (第0块保留)
    int block_id = -1;
    for (int i = 1; i < MAX_BLOCKS; i++) {
        if (!bitmap_test_bit(g_fs.block_bitmap, i)) {
            block_id = i;
            break;
        }
    }
    
    if (block_id == -1) {
        printf("错误: 没有空闲数据块\n");
        return -1;
    }
    
    // 标记为已使用
    bitmap_set_bit(g_fs.block_bitmap, block_id);
    
    // 清空块内容
    block_clear(block_id);
    
    // 更新超级块统计
    g_fs.superblock.free_blocks--;
    g_fs.is_dirty = true;
    
    return block_id;
}

/**
 * 释放一个数据块
 */
void block_free(int block_id) {
    if (block_id <= 0 || block_id >= MAX_BLOCKS) {
        return;  // 保护第0块和无效块
    }
    
    // 清除位图标记
    bitmap_clear_bit(g_fs.block_bitmap, block_id);
    
    // 清空块内容
    block_clear(block_id);
    
    // 更新超级块统计
    g_fs.superblock.free_blocks++;
    g_fs.is_dirty = true;
}

/**
 * 读取数据块内容
 */
int block_read(int block_id, void *buffer) {
    if (block_id < 0 || block_id >= MAX_BLOCKS || !buffer) {
        return -1;
    }
    
    SuperBlock *sb = &g_fs.superblock;
    off_t offset = sb->data_blocks_offset + block_id * BLOCK_SIZE;
    
    return disk_read(offset, buffer, BLOCK_SIZE);
}

/**
 * 写入数据块内容
 */
int block_write(int block_id, const void *buffer) {
    if (block_id < 0 || block_id >= MAX_BLOCKS || !buffer) {
        return -1;
    }
    
    SuperBlock *sb = &g_fs.superblock;
    off_t offset = sb->data_blocks_offset + block_id * BLOCK_SIZE;
    
    return disk_write(offset, buffer, BLOCK_SIZE);
}

/**
 * 检查数据块是否被使用
 */
bool block_is_used(int block_id) {
    if (block_id < 0 || block_id >= MAX_BLOCKS) {
        return false;
    }
    
    return bitmap_test_bit(g_fs.block_bitmap, block_id);
}

/**
 * 清空数据块内容
 */
int block_clear(int block_id) {
    if (block_id < 0 || block_id >= MAX_BLOCKS) {
        return -1;
    }
    
    char zero_buffer[BLOCK_SIZE];
    memset(zero_buffer, 0, BLOCK_SIZE);
    
    return block_write(block_id, zero_buffer);
}

/**
 * 为inode分配数据块
 */
int block_alloc_for_inode(int inode_id) {
    if (!inode_is_used(inode_id)) {
        return -1;
    }
    
    Inode *inode = &g_fs.inode_table[inode_id];
    
    // 检查是否还有空间分配新块
    if (inode->block_count >= MAX_DIRECT_BLOCKS) {
        printf("错误: inode %d 已达到最大数据块数\n", inode_id);
        return -1;
    }
    
    // 分配新块
    int block_id = block_alloc();
    if (block_id == -1) {
        return -1;
    }
    
    // 添加到inode的数据块列表
    inode->data_blocks[inode->block_count] = block_id;
    inode->block_count++;
    
    g_fs.is_dirty = true;
    return block_id;
}

/**
 * 释放inode的所有数据块
 */
int block_free_all_for_inode(int inode_id) {
    if (!inode_is_used(inode_id)) {
        return -1;
    }
    
    Inode *inode = &g_fs.inode_table[inode_id];
    
    // 释放所有数据块
    for (int i = 0; i < inode->block_count && i < MAX_DIRECT_BLOCKS; i++) {
        if (inode->data_blocks[i] > 0) {
            block_free(inode->data_blocks[i]);
            inode->data_blocks[i] = 0;
        }
    }
    
    inode->block_count = 0;
    inode->size = 0;
    
    g_fs.is_dirty = true;
    return 0;
}

/**
 * 获取inode的第n个数据块
 */
int block_get_for_inode(int inode_id, int block_index) {
    if (!inode_is_used(inode_id) || block_index < 0) {
        return -1;
    }
    
    Inode *inode = &g_fs.inode_table[inode_id];
    
    if (block_index >= inode->block_count || block_index >= MAX_DIRECT_BLOCKS) {
        return -1;
    }
    
    return inode->data_blocks[block_index];
}

/**
 * 计算需要的数据块数量
 */
int block_count_needed(size_t size) {
    if (size == 0) {
        return 0;
    }
    
    return (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
}

/**
 * 打印数据块使用统计
 */
void block_print_usage(void) {
    int used = bitmap_count_used_bits(g_fs.block_bitmap, MAX_BLOCKS);
    int free = MAX_BLOCKS - used;
    
    printf("\n=== 数据块使用统计 ===\n");
    printf("总数据块数: %d\n", MAX_BLOCKS);
    printf("已使用: %d\n", used);
    printf("空闲: %d\n", free);
    printf("使用率: %.1f%%\n", (float)used / MAX_BLOCKS * 100);
    printf("块大小: %d 字节\n", BLOCK_SIZE);
    printf("总容量: %.1f KB\n", (float)(MAX_BLOCKS * BLOCK_SIZE) / 1024);
    printf("已使用容量: %.1f KB\n", (float)(used * BLOCK_SIZE) / 1024);
    printf("空闲容量: %.1f KB\n", (float)(free * BLOCK_SIZE) / 1024);
    printf("====================\n\n");
}

/**
 * 打印数据块信息 (调试用)
 */
void block_print_info(int block_id) {
    if (block_id < 0 || block_id >= MAX_BLOCKS) {
        printf("数据块 %d: 无效\n", block_id);
        return;
    }
    
    bool used = block_is_used(block_id);
    SuperBlock *sb = &g_fs.superblock;
    off_t offset = sb->data_blocks_offset + block_id * BLOCK_SIZE;
    
    printf("\n=== 数据块 %d 信息 ===\n", block_id);
    printf("状态: %s\n", used ? "已使用" : "空闲");
    printf("大小: %d 字节\n", BLOCK_SIZE);
    printf("磁盘偏移: %ld\n", offset);
    
    if (used) {
        // 读取并显示块的前64字节内容
        char buffer[BLOCK_SIZE];
        if (block_read(block_id, buffer) > 0) {
            printf("内容预览 (前64字节):\n");
            for (int i = 0; i < 64 && i < BLOCK_SIZE; i++) {
                if (i % 16 == 0) printf("%04x: ", i);
                printf("%02x ", (unsigned char)buffer[i]);
                if (i % 16 == 15) printf("\n");
            }
            if (64 % 16 != 0) printf("\n");
        }
    }
    
    printf("===================\n\n");
}
