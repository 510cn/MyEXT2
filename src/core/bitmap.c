/*
 * ============================================================================
 * 文件名: src/core/bitmap.c
 * 描述: 位图管理模块实现
 * 功能: inode和数据块位图的管理
 * ============================================================================
 */

#include "bitmap.h"
#include "disk.h"

// ============================================================================
// 位图管理函数实现
// ============================================================================

/**
 * 初始化位图
 */
int bitmap_init(void) {
    // 分配inode位图内存
    g_fs.inode_bitmap = calloc(MAX_INODES / 8, 1);
    if (!g_fs.inode_bitmap) {
        printf("错误: 无法分配inode位图内存\n");
        return -1;
    }
    
    // 分配数据块位图内存
    g_fs.block_bitmap = calloc(MAX_BLOCKS / 8, 1);
    if (!g_fs.block_bitmap) {
        printf("错误: 无法分配数据块位图内存\n");
        free(g_fs.inode_bitmap);
        return -1;
    }
    
    // 标记根目录使用的资源
    bitmap_set_bit(g_fs.inode_bitmap, ROOT_INODE);  // 根目录inode
    bitmap_set_bit(g_fs.block_bitmap, 0);           // 用户信息块
    
    printf("位图初始化完成\n");
    return 0;
}

/**
 * 加载位图从磁盘
 */
int bitmap_load(void) {
    SuperBlock *sb = &g_fs.superblock;
    
    // 分配内存
    if (bitmap_init() != 0) {
        return -1;
    }
    
    // 读取inode位图
    if (disk_read(sb->inode_bitmap_offset, g_fs.inode_bitmap, MAX_INODES / 8) != 0) {
        printf("错误: 无法读取inode位图\n");
        return -1;
    }
    
    // 读取数据块位图
    if (disk_read(sb->block_bitmap_offset, g_fs.block_bitmap, MAX_BLOCKS / 8) != 0) {
        printf("错误: 无法读取数据块位图\n");
        return -1;
    }
    
    printf("位图加载完成\n");
    return 0;
}

/**
 * 保存位图到磁盘
 */
int bitmap_save(void) {
    SuperBlock *sb = &g_fs.superblock;
    
    // 写入inode位图
    if (disk_write(sb->inode_bitmap_offset, g_fs.inode_bitmap, MAX_INODES / 8) != 0) {
        printf("错误: 无法写入inode位图\n");
        return -1;
    }
    
    // 写入数据块位图
    if (disk_write(sb->block_bitmap_offset, g_fs.block_bitmap, MAX_BLOCKS / 8) != 0) {
        printf("错误: 无法写入数据块位图\n");
        return -1;
    }
    
    return 0;
}

/**
 * 设置位图中的某一位
 */
int bitmap_set_bit(char *bitmap, int bit) {
    if (bit < 0) {
        return -1;
    }
    
    int byte_index = bit / 8;
    int bit_index = bit % 8;
    
    bitmap[byte_index] |= (1 << bit_index);
    return 0;
}

/**
 * 清除位图中的某一位
 */
int bitmap_clear_bit(char *bitmap, int bit) {
    if (bit < 0) {
        return -1;
    }
    
    int byte_index = bit / 8;
    int bit_index = bit % 8;
    
    bitmap[byte_index] &= ~(1 << bit_index);
    return 0;
}

/**
 * 测试位图中的某一位是否被设置
 */
bool bitmap_test_bit(const char *bitmap, int bit) {
    if (bit < 0) {
        return false;
    }
    
    int byte_index = bit / 8;
    int bit_index = bit % 8;
    
    return (bitmap[byte_index] & (1 << bit_index)) != 0;
}

/**
 * 在位图中查找第一个空闲位
 */
int bitmap_find_free_bit(const char *bitmap, int max_bits) {
    for (int i = 0; i < max_bits; i++) {
        if (!bitmap_test_bit(bitmap, i)) {
            return i;
        }
    }
    return -1;  // 没有找到空闲位
}

/**
 * 统计位图中已使用的位数
 */
int bitmap_count_used_bits(const char *bitmap, int max_bits) {
    int count = 0;
    for (int i = 0; i < max_bits; i++) {
        if (bitmap_test_bit(bitmap, i)) {
            count++;
        }
    }
    return count;
}

/**
 * 统计位图中空闲的位数
 */
int bitmap_count_free_bits(const char *bitmap, int max_bits) {
    return max_bits - bitmap_count_used_bits(bitmap, max_bits);
}

/**
 * 打印位图状态 (调试用)
 */
void bitmap_print_status(const char *bitmap, int max_bits, const char *name) {
    int used = bitmap_count_used_bits(bitmap, max_bits);
    int free = max_bits - used;
    
    printf("%s位图状态:\n", name);
    printf("- 总数: %d\n", max_bits);
    printf("- 已使用: %d\n", used);
    printf("- 空闲: %d\n", free);
    printf("- 使用率: %.1f%%\n", (float)used / max_bits * 100);
    
    // 打印位图的前32位状态 (调试用)
    printf("- 前32位状态: ");
    for (int i = 0; i < 32 && i < max_bits; i++) {
        printf("%c", bitmap_test_bit(bitmap, i) ? '1' : '0');
        if ((i + 1) % 8 == 0) printf(" ");
    }
    printf("\n");
}
