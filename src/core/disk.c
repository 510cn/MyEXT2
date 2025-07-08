/*
 * ============================================================================
 * 文件名: src/core/disk.c
 * 描述: 磁盘I/O模块实现
 * 功能: 虚拟磁盘文件的读写操作
 * ============================================================================
 */

#include "disk.h"

// ============================================================================
// 静态变量
// ============================================================================
static struct {
    FILE *file;                         // 磁盘文件句柄
    char path[256];                     // 磁盘文件路径
    off_t size;                         // 磁盘大小
    uint64_t read_count;                // 读取次数统计
    uint64_t write_count;               // 写入次数统计
    uint64_t bytes_read;                // 读取字节数统计
    uint64_t bytes_written;             // 写入字节数统计
} disk_state = {0};

// ============================================================================
// 磁盘I/O函数实现
// ============================================================================

/**
 * 初始化磁盘I/O系统
 */
int disk_init(const char *image_path) {
    // 检查文件是否存在
    FILE *test_file = fopen(image_path, "rb");
    if (test_file) {
        fclose(test_file);
        printf("发现现有磁盘镜像，正在加载...\n");
        return disk_open(image_path);
    } else {
        printf("磁盘镜像不存在，正在创建...\n");
        return disk_create(image_path);
    }
}

/**
 * 创建新的磁盘镜像文件
 */
int disk_create(const char *image_path) {
    // 创建新文件
    disk_state.file = fopen(image_path, "wb+");
    if (!disk_state.file) {
        printf("错误: 无法创建磁盘镜像文件 %s\n", image_path);
        return -1;
    }
    
    // 预分配磁盘空间
    off_t disk_size = MAX_BLOCKS * BLOCK_SIZE;
    if (fseek(disk_state.file, disk_size - 1, SEEK_SET) != 0) {
        printf("错误: 无法设置磁盘大小\n");
        fclose(disk_state.file);
        return -1;
    }
    
    // 写入最后一个字节以确保文件大小
    if (fputc(0, disk_state.file) == EOF) {
        printf("错误: 无法写入磁盘文件\n");
        fclose(disk_state.file);
        return -1;
    }
    
    // 回到文件开头
    rewind(disk_state.file);
    
    // 保存状态
    strncpy(disk_state.path, image_path, sizeof(disk_state.path) - 1);
    disk_state.size = disk_size;
    disk_state.read_count = 0;
    disk_state.write_count = 0;
    disk_state.bytes_read = 0;
    disk_state.bytes_written = 0;
    
    printf("磁盘镜像创建完成: %s (大小: %ld 字节)\n", image_path, disk_size);
    return 0;
}

/**
 * 打开现有的磁盘镜像文件
 */
int disk_open(const char *image_path) {
    // 打开文件
    disk_state.file = fopen(image_path, "rb+");
    if (!disk_state.file) {
        printf("错误: 无法打开磁盘镜像文件 %s\n", image_path);
        return -1;
    }
    
    // 获取文件大小
    if (fseek(disk_state.file, 0, SEEK_END) != 0) {
        printf("错误: 无法获取磁盘文件大小\n");
        fclose(disk_state.file);
        return -1;
    }
    
    disk_state.size = ftell(disk_state.file);
    rewind(disk_state.file);
    
    // 保存状态
    strncpy(disk_state.path, image_path, sizeof(disk_state.path) - 1);
    disk_state.read_count = 0;
    disk_state.write_count = 0;
    disk_state.bytes_read = 0;
    disk_state.bytes_written = 0;
    
    printf("磁盘镜像加载完成: %s (大小: %ld 字节)\n", image_path, disk_state.size);
    return 0;
}

/**
 * 从磁盘读取数据
 */
int disk_read(off_t offset, void *buffer, size_t size) {
    if (!disk_state.file) {
        printf("错误: 磁盘未初始化\n");
        return -1;
    }
    
    if (offset < 0 || offset >= disk_state.size) {
        printf("错误: 读取偏移量超出范围 (偏移: %ld, 大小: %ld)\n", offset, disk_state.size);
        return -1;
    }
    
    // 定位到指定位置
    if (fseek(disk_state.file, offset, SEEK_SET) != 0) {
        printf("错误: 无法定位到偏移量 %ld\n", offset);
        return -1;
    }
    
    // 读取数据
    size_t bytes_read = fread(buffer, 1, size, disk_state.file);
    
    // 更新统计信息
    disk_state.read_count++;
    disk_state.bytes_read += bytes_read;
    
    return bytes_read;
}

/**
 * 向磁盘写入数据
 */
int disk_write(off_t offset, const void *buffer, size_t size) {
    if (!disk_state.file) {
        printf("错误: 磁盘未初始化\n");
        return -1;
    }
    
    if (offset < 0 || offset >= disk_state.size) {
        printf("错误: 写入偏移量超出范围 (偏移: %ld, 大小: %ld)\n", offset, disk_state.size);
        return -1;
    }
    
    // 定位到指定位置
    if (fseek(disk_state.file, offset, SEEK_SET) != 0) {
        printf("错误: 无法定位到偏移量 %ld\n", offset);
        return -1;
    }
    
    // 写入数据
    size_t bytes_written = fwrite(buffer, 1, size, disk_state.file);

    // 检查写入是否完整
    if (bytes_written != size) {
        printf("错误: 写入不完整 (期望: %zu, 实际: %zu)\n", size, bytes_written);
        return -1;
    }

    // 强制刷新到磁盘
    fflush(disk_state.file);

    // 更新统计信息
    disk_state.write_count++;
    disk_state.bytes_written += bytes_written;

    // 标记文件系统为脏
    g_fs.is_dirty = true;

    return 0;  // 成功返回0
}

/**
 * 同步磁盘数据 (强制写入)
 */
int disk_sync(void) {
    if (!disk_state.file) {
        return -1;
    }
    
    if (fflush(disk_state.file) != 0) {
        printf("错误: 无法同步磁盘数据\n");
        return -1;
    }
    
    return 0;
}

/**
 * 获取磁盘大小
 */
off_t disk_get_size(void) {
    return disk_state.size;
}

/**
 * 检查磁盘是否已打开
 */
bool disk_is_open(void) {
    return disk_state.file != NULL;
}

/**
 * 清理磁盘I/O系统
 */
void disk_cleanup(void) {
    if (disk_state.file) {
        // 同步数据
        disk_sync();
        
        // 关闭文件
        fclose(disk_state.file);
        disk_state.file = NULL;
        
        printf("磁盘I/O系统已清理\n");
    }
}

/**
 * 打印磁盘统计信息
 */
void disk_print_stats(void) {
    printf("\n=== 磁盘I/O统计 ===\n");
    printf("磁盘文件: %s\n", disk_state.path);
    printf("磁盘大小: %ld 字节 (%.1f KB)\n", 
           disk_state.size, (double)disk_state.size / 1024);
    printf("读取次数: %lu\n", disk_state.read_count);
    printf("写入次数: %lu\n", disk_state.write_count);
    printf("读取字节: %lu (%.1f KB)\n", 
           disk_state.bytes_read, (double)disk_state.bytes_read / 1024);
    printf("写入字节: %lu (%.1f KB)\n", 
           disk_state.bytes_written, (double)disk_state.bytes_written / 1024);
    printf("==================\n\n");
}
