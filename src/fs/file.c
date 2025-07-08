/*
 * ============================================================================
 * 文件名: src/fs/file.c
 * 描述: 文件操作模块实现
 * 功能: 文件的创建、删除、读写和管理
 * ============================================================================
 */

#include "file.h"
#include "inode.h"
#include "block.h"
#include "directory.h"

// ============================================================================
// 文件操作函数实现
// ============================================================================

/**
 * 创建文件
 */
int file_create(const char *name, int parent_inode) {
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
    int new_inode = inode_create(name, false, parent_inode);
    if (new_inode == -1) {
        return -1;
    }
    
    return new_inode;
}

/**
 * 删除文件
 */
int file_delete(int inode_id) {
    if (!inode_is_used(inode_id) || g_fs.inode_table[inode_id].is_directory) {
        return -1;  // 不是文件
    }
    
    // 释放所有数据块
    block_free_all_for_inode(inode_id);
    
    // 释放inode
    inode_free(inode_id);
    
    return 0;
}

/**
 * 读取文件内容
 */
int file_read(int inode_id, void *buffer, size_t size, off_t offset) {
    if (!inode_is_used(inode_id) || g_fs.inode_table[inode_id].is_directory || !buffer) {
        return -1;
    }
    
    Inode *inode = &g_fs.inode_table[inode_id];
    
    // 检查偏移量
    if (offset >= inode->size) {
        return 0;  // 超出文件末尾
    }
    
    // 调整读取大小
    if (offset + size > (size_t)inode->size) {
        size = inode->size - offset;
    }
    
    size_t bytes_read = 0;
    char *buf = (char *)buffer;
    
    // 逐块读取
    while (bytes_read < size) {
        // 计算当前块索引和块内偏移
        int block_index = (offset + bytes_read) / BLOCK_SIZE;
        int block_offset = (offset + bytes_read) % BLOCK_SIZE;
        
        // 获取数据块编号
        int block_id = block_get_for_inode(inode_id, block_index);
        if (block_id == -1) {
            break;  // 没有更多数据块
        }
        
        // 读取块数据
        char block_buffer[BLOCK_SIZE];
        if (block_read(block_id, block_buffer) < 0) {
            break;  // 读取失败
        }
        
        // 计算本次读取的字节数
        size_t to_read = BLOCK_SIZE - block_offset;
        if (to_read > size - bytes_read) {
            to_read = size - bytes_read;
        }
        
        // 复制数据
        memcpy(buf + bytes_read, block_buffer + block_offset, to_read);
        bytes_read += to_read;
    }
    
    // 更新访问时间
    inode_update_times(inode_id, true, false);
    
    return bytes_read;
}

/**
 * 写入文件内容
 */
int file_write(int inode_id, const void *buffer, size_t size, off_t offset) {
    if (!inode_is_used(inode_id) || g_fs.inode_table[inode_id].is_directory || !buffer) {
        return -1;
    }
    
    Inode *inode = &g_fs.inode_table[inode_id];
    const char *buf = (const char *)buffer;
    size_t bytes_written = 0;
    
    // 确保有足够的数据块
    size_t new_size = offset + size;
    if (file_alloc_blocks(inode_id, new_size) != 0) {
        return -1;  // 分配失败
    }
    
    // 逐块写入
    while (bytes_written < size) {
        // 计算当前块索引和块内偏移
        int block_index = (offset + bytes_written) / BLOCK_SIZE;
        int block_offset = (offset + bytes_written) % BLOCK_SIZE;
        
        // 获取数据块编号
        int block_id = block_get_for_inode(inode_id, block_index);
        if (block_id == -1) {
            // 需要分配新块
            block_id = block_alloc_for_inode(inode_id);
            if (block_id == -1) {
                break;  // 分配失败
            }
        }
        
        // 读取现有块数据 (如果需要部分写入)
        char block_buffer[BLOCK_SIZE];
        if (block_offset != 0 || (size - bytes_written) < BLOCK_SIZE) {
            if (block_read(block_id, block_buffer) < 0) {
                memset(block_buffer, 0, BLOCK_SIZE);  // 清零
            }
        }
        
        // 计算本次写入的字节数
        size_t to_write = BLOCK_SIZE - block_offset;
        if (to_write > size - bytes_written) {
            to_write = size - bytes_written;
        }
        
        // 复制数据到块缓冲区
        memcpy(block_buffer + block_offset, buf + bytes_written, to_write);
        
        // 写入块
        if (block_write(block_id, block_buffer) < 0) {
            break;  // 写入失败
        }
        
        bytes_written += to_write;
    }
    
    // 更新文件大小
    if (offset + bytes_written > (size_t)inode->size) {
        inode->size = offset + bytes_written;
    }
    
    // 更新修改时间
    inode_update_times(inode_id, false, true);
    
    return bytes_written;
}

/**
 * 截断文件
 */
int file_truncate(int inode_id, off_t size) {
    if (!inode_is_used(inode_id) || g_fs.inode_table[inode_id].is_directory) {
        return -1;
    }
    
    Inode *inode = &g_fs.inode_table[inode_id];
    
    if (size < 0) {
        return -1;  // 无效大小
    }
    
    if (size == 0) {
        // 截断为0，释放所有数据块
        block_free_all_for_inode(inode_id);
        inode->size = 0;
    } else if ((size_t)size < inode->size) {
        // 缩小文件，释放多余的块
        file_free_excess_blocks(inode_id, size);
        inode->size = size;
    } else if ((size_t)size > inode->size) {
        // 扩大文件，分配新块
        if (file_alloc_blocks(inode_id, size) == 0) {
            inode->size = size;
        } else {
            return -1;  // 分配失败
        }
    }
    
    // 更新修改时间
    inode_update_times(inode_id, false, true);
    
    return 0;
}

/**
 * 重命名文件
 */
int file_rename(int inode_id, const char *new_name) {
    return inode_rename(inode_id, new_name);
}

/**
 * 获取文件大小
 */
off_t file_get_size(int inode_id) {
    if (!inode_is_used(inode_id) || g_fs.inode_table[inode_id].is_directory) {
        return -1;
    }
    
    return g_fs.inode_table[inode_id].size;
}

/**
 * 设置文件大小
 */
int file_set_size(int inode_id, off_t size) {
    return file_truncate(inode_id, size);
}

/**
 * 检查文件是否存在
 */
int file_exists(int parent_inode, const char *name) {
    int inode_id = dir_find_entry(parent_inode, name);
    if (inode_id != -1 && !g_fs.inode_table[inode_id].is_directory) {
        return inode_id;
    }
    return -1;
}

/**
 * 分配文件所需的数据块
 */
int file_alloc_blocks(int inode_id, size_t size) {
    if (!inode_is_used(inode_id)) {
        return -1;
    }
    
    Inode *inode = &g_fs.inode_table[inode_id];
    int needed_blocks = block_count_needed(size);
    
    // 分配不足的块
    while (inode->block_count < needed_blocks && inode->block_count < MAX_DIRECT_BLOCKS) {
        int block_id = block_alloc_for_inode(inode_id);
        if (block_id == -1) {
            return -1;  // 分配失败
        }
    }
    
    return 0;
}

/**
 * 释放文件多余的数据块
 */
int file_free_excess_blocks(int inode_id, size_t new_size) {
    if (!inode_is_used(inode_id)) {
        return -1;
    }
    
    Inode *inode = &g_fs.inode_table[inode_id];
    int needed_blocks = block_count_needed(new_size);
    
    // 释放多余的块
    while (inode->block_count > needed_blocks) {
        int last_block = inode->block_count - 1;
        if (last_block >= 0 && last_block < MAX_DIRECT_BLOCKS) {
            block_free(inode->data_blocks[last_block]);
            inode->data_blocks[last_block] = 0;
            inode->block_count--;
        } else {
            break;
        }
    }
    
    return 0;
}

/**
 * 打印文件信息 (调试用)
 */
void file_print_info(int inode_id) {
    if (!inode_is_used(inode_id)) {
        printf("文件 %d: 不存在\n", inode_id);
        return;
    }
    
    if (g_fs.inode_table[inode_id].is_directory) {
        printf("文件 %d: 这是一个目录\n", inode_id);
        return;
    }
    
    Inode *inode = &g_fs.inode_table[inode_id];
    
    printf("\n=== 文件 %d 信息 ===\n", inode_id);
    printf("名称: %s\n", inode->name);
    printf("大小: %u 字节\n", inode->size);
    printf("数据块数: %u\n", inode->block_count);
    printf("所有者: %u\n", inode->owner_id);
    printf("权限: %s\n", inode->permissions);
    printf("父目录: %u\n", inode->parent_inode);
    printf("硬链接数: %u\n", inode->link_count);
    printf("创建时间: %s", ctime(&inode->created));
    printf("修改时间: %s", ctime(&inode->modified));
    printf("访问时间: %s", ctime(&inode->accessed));
    
    printf("数据块列表: ");
    for (int i = 0; i < inode->block_count && i < MAX_DIRECT_BLOCKS; i++) {
        printf("%u ", inode->data_blocks[i]);
    }
    printf("\n");
    
    printf("完整性: %s\n", file_verify_integrity(inode_id) ? "正常" : "损坏");
    printf("==================\n\n");
}

/**
 * 验证文件完整性
 */
bool file_verify_integrity(int inode_id) {
    if (!inode_is_used(inode_id) || g_fs.inode_table[inode_id].is_directory) {
        return false;
    }
    
    Inode *inode = &g_fs.inode_table[inode_id];
    
    // 检查数据块是否有效
    for (int i = 0; i < inode->block_count && i < MAX_DIRECT_BLOCKS; i++) {
        int block_id = inode->data_blocks[i];
        if (block_id <= 0 || block_id >= MAX_BLOCKS || !block_is_used(block_id)) {
            return false;  // 无效的数据块
        }
    }
    
    // 检查文件大小与块数的一致性
    int expected_blocks = block_count_needed(inode->size);
    if (inode->block_count < expected_blocks) {
        return false;  // 块数不足
    }
    
    return true;
}
