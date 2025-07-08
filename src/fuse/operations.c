/*
 * ============================================================================
 * 文件名: src/fuse/operations.c
 * 描述: FUSE操作接口实现
 * 功能: 将FUSE调用映射到文件系统核心函数
 * ============================================================================
 */

#include "operations.h"
#include "../fs/superblock.h"
#include "../fs/inode.h"
#include "../fs/block.h"
#include "../fs/directory.h"
#include "../fs/file.h"
#include "../core/disk.h"
#include "../core/bitmap.h"

// ============================================================================
// 全局变量定义
// ============================================================================
FileSystem g_fs = {0};

// ============================================================================
// 辅助函数实现
// ============================================================================

/**
 * 解析父目录路径和文件名
 */
static int parse_parent_path(const char *path, char *filename) {
    char *path_copy = strdup(path);
    char *last_slash = strrchr(path_copy, '/');
    
    if (last_slash == path_copy) {
        // 父目录是根目录
        strcpy(filename, last_slash + 1);
        free(path_copy);
        return ROOT_INODE;
    } else {
        // 分离父目录和文件名
        *last_slash = '\0';
        strcpy(filename, last_slash + 1);
        
        int parent_inode = dir_resolve_path(path_copy);
        free(path_copy);
        return parent_inode;
    }
}

/**
 * 将POSIX错误码转换为FUSE错误码
 */
static int errno_to_fuse_error(ext2fs_error_t error) {
    switch (error) {
        case EXT2FS_SUCCESS: return 0;
        case EXT2FS_ERROR_NOT_FOUND: return -ENOENT;
        case EXT2FS_ERROR_EXISTS: return -EEXIST;
        case EXT2FS_ERROR_NO_SPACE: return -ENOSPC;
        case EXT2FS_ERROR_PERMISSION: return -EPERM;
        case EXT2FS_ERROR_NOT_DIR: return -ENOTDIR;
        case EXT2FS_ERROR_IS_DIR: return -EISDIR;
        case EXT2FS_ERROR_NOT_EMPTY: return -ENOTEMPTY;
        case EXT2FS_ERROR_IO: return -EIO;
        case EXT2FS_ERROR_INVALID: return -EINVAL;
        default: return -EIO;
    }
}

/**
 * 将权限模式转换为权限字符串
 */
static void mode_to_permissions(mode_t mode, char *perms) {
    strcpy(perms, "");
    if (mode & S_IRUSR) strcat(perms, "r");
    if (mode & S_IWUSR) strcat(perms, "w");
    if (mode & S_IXUSR) strcat(perms, "x");
}

/**
 * 将权限字符串转换为权限模式
 */
static mode_t permissions_to_mode(const char *perms, bool is_dir) {
    mode_t mode = 0;
    
    if (is_dir) {
        mode |= S_IFDIR;
    } else {
        mode |= S_IFREG;
    }
    
    if (strchr(perms, 'r')) mode |= S_IRUSR | S_IRGRP | S_IROTH;
    if (strchr(perms, 'w')) mode |= S_IWUSR | S_IWGRP | S_IWOTH;
    if (strchr(perms, 'x')) mode |= S_IXUSR | S_IXGRP | S_IXOTH;
    
    return mode;
}

// ============================================================================
// FUSE操作函数实现
// ============================================================================

/**
 * 获取文件属性
 */
static int fuse_getattr(const char *path, struct stat *stbuf) {
    int inode_id = dir_resolve_path(path);
    if (inode_id == -1) {
        return -ENOENT;
    }
    
    return inode_get_stat(inode_id, stbuf);
}

/**
 * 读取目录内容
 */
static int fuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                        off_t offset, struct fuse_file_info *fi) {
    (void) offset;
    (void) fi;

    int dir_inode = dir_resolve_path(path);
    if (dir_inode == -1) {
        return -ENOENT;
    }

    if (!g_fs.inode_table[dir_inode].is_directory) {
        return -ENOTDIR;
    }

    // 添加 . 和 .. 目录项
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    // 添加子文件和子目录 (修复：避免目录自己列出自己)
    for (int i = 0; i < MAX_INODES; i++) {
        if (inode_is_used(i) && g_fs.inode_table[i].parent_inode == dir_inode && i != dir_inode) {
            filler(buf, g_fs.inode_table[i].name, NULL, 0);
        }
    }

    return 0;
}

/**
 * 打开文件
 */
static int fuse_open(const char *path, struct fuse_file_info *fi) {
    int inode_id = dir_resolve_path(path);
    if (inode_id == -1) {
        return -ENOENT;
    }
    
    if (g_fs.inode_table[inode_id].is_directory) {
        return -EISDIR;
    }
    
    fi->fh = inode_id;
    return 0;
}

/**
 * 读取文件内容
 */
static int fuse_read(const char *path, char *buf, size_t size, off_t offset,
                     struct fuse_file_info *fi) {
    int inode_id = dir_resolve_path(path);
    if (inode_id == -1) {
        return -ENOENT;
    }
    
    int bytes_read = file_read(inode_id, buf, size, offset);
    if (bytes_read < 0) {
        return -EIO;
    }
    
    return bytes_read;
}

/**
 * 写入文件内容
 */
static int fuse_write(const char *path, const char *buf, size_t size,
                      off_t offset, struct fuse_file_info *fi) {
    int inode_id = dir_resolve_path(path);
    if (inode_id == -1) {
        return -ENOENT;
    }
    
    int bytes_written = file_write(inode_id, buf, size, offset);
    if (bytes_written < 0) {
        return -EIO;
    }
    
    return bytes_written;
}

/**
 * 创建文件
 */
static int fuse_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    (void) mode;  // 忽略权限模式，使用默认
    
    char filename[MAX_FILENAME];
    int parent_inode = parse_parent_path(path, filename);
    
    if (parent_inode == -1) {
        return -ENOENT;
    }
    
    int new_inode = file_create(filename, parent_inode);
    if (new_inode == -1) {
        return -ENOSPC;
    }
    
    fi->fh = new_inode;
    return 0;
}

/**
 * 创建目录
 */
static int fuse_mkdir(const char *path, mode_t mode) {
    (void) mode;  // 忽略权限模式，使用默认
    
    char dirname[MAX_FILENAME];
    int parent_inode = parse_parent_path(path, dirname);
    
    if (parent_inode == -1) {
        return -ENOENT;
    }
    
    int new_inode = dir_create(dirname, parent_inode);
    if (new_inode == -1) {
        return -ENOSPC;
    }
    
    return 0;
}

/**
 * 删除文件
 */
static int fuse_unlink(const char *path) {
    int inode_id = dir_resolve_path(path);
    if (inode_id == -1) {
        return -ENOENT;
    }
    
    if (g_fs.inode_table[inode_id].is_directory) {
        return -EISDIR;
    }
    
    int result = file_delete(inode_id);
    if (result == -1) {
        return -EIO;
    }
    
    return 0;
}

/**
 * 删除目录
 */
static int fuse_rmdir(const char *path) {
    int inode_id = dir_resolve_path(path);
    if (inode_id == -1) {
        return -ENOENT;
    }
    
    if (!g_fs.inode_table[inode_id].is_directory) {
        return -ENOTDIR;
    }
    
    if (!dir_is_empty(inode_id)) {
        return -ENOTEMPTY;
    }
    
    int result = dir_delete(inode_id);
    if (result == -1) {
        return -EIO;
    }
    
    return 0;
}

/**
 * 重命名文件/目录
 */
static int fuse_rename(const char *from, const char *to) {
    int from_inode = dir_resolve_path(from);
    if (from_inode == -1) {
        return -ENOENT;
    }
    
    char new_name[MAX_FILENAME];
    int to_parent = parse_parent_path(to, new_name);
    if (to_parent == -1) {
        return -ENOENT;
    }
    
    // 简化：只支持在同一目录内重命名
    if (g_fs.inode_table[from_inode].parent_inode != to_parent) {
        return -EXDEV;
    }
    
    int result;
    if (g_fs.inode_table[from_inode].is_directory) {
        result = inode_rename(from_inode, new_name);
    } else {
        result = file_rename(from_inode, new_name);
    }
    
    if (result == -1) {
        return -EIO;
    }
    
    return 0;
}

/**
 * 修改文件权限
 */
static int fuse_chmod(const char *path, mode_t mode) {
    int inode_id = dir_resolve_path(path);
    if (inode_id == -1) {
        return -ENOENT;
    }
    
    char perms[4];
    mode_to_permissions(mode, perms);
    
    int result = inode_set_permissions(inode_id, perms);
    if (result == -1) {
        return -EPERM;
    }
    
    return 0;
}

/**
 * 截断文件
 */
static int fuse_truncate(const char *path, off_t size) {
    int inode_id = dir_resolve_path(path);
    if (inode_id == -1) {
        return -ENOENT;
    }
    
    int result = file_truncate(inode_id, size);
    if (result == -1) {
        return -EIO;
    }
    
    return 0;
}

/**
 * 更新文件时间
 */
static int fuse_utimens(const char *path, const struct timespec ts[2]) {
    int inode_id = dir_resolve_path(path);
    if (inode_id == -1) {
        return -ENOENT;
    }

    if (ts && ts[0].tv_nsec != UTIME_OMIT) {
        g_fs.inode_table[inode_id].accessed = ts[0].tv_sec;
    }
    if (ts && ts[1].tv_nsec != UTIME_OMIT) {
        g_fs.inode_table[inode_id].modified = ts[1].tv_sec;
    }

    g_fs.is_dirty = true;
    return 0;
}

/**
 * 获取文件系统统计信息
 */
static int fuse_statfs(const char *path, struct statvfs *stbuf) {
    (void) path;  // 忽略路径参数

    memset(stbuf, 0, sizeof(struct statvfs));

    stbuf->f_bsize = BLOCK_SIZE;
    stbuf->f_frsize = BLOCK_SIZE;
    stbuf->f_blocks = g_fs.superblock.total_blocks;
    stbuf->f_bfree = g_fs.superblock.free_blocks;
    stbuf->f_bavail = g_fs.superblock.free_blocks;
    stbuf->f_files = g_fs.superblock.total_inodes;
    stbuf->f_ffree = g_fs.superblock.free_inodes;
    stbuf->f_namemax = MAX_FILENAME;

    return 0;
}

/**
 * FUSE初始化
 */
static void *fuse_init(struct fuse_conn_info *conn) {
    (void) conn;  // 忽略连接信息

    printf("正在初始化模块化EXT2文件系统...\n");

    // 检查是否为新文件系统
    FILE *test_file = fopen(DISK_IMAGE, "rb");
    bool is_new_fs = (test_file == NULL);
    if (test_file) fclose(test_file);

    // 初始化磁盘I/O
    if (disk_init(DISK_IMAGE) != 0) {
        printf("错误: 磁盘初始化失败\n");
        return NULL;
    }

    if (is_new_fs) {
        printf("创建新的文件系统...\n");

        // 初始化超级块
        if (superblock_init() != 0) {
            printf("错误: 超级块初始化失败\n");
            return NULL;
        }

        // 初始化位图
        if (bitmap_init() != 0) {
            printf("错误: 位图初始化失败\n");
            return NULL;
        }

        // 初始化inode表
        if (inode_init() != 0) {
            printf("错误: inode表初始化失败\n");
            return NULL;
        }

        // 初始化数据块管理
        if (block_init() != 0) {
            printf("错误: 数据块管理初始化失败\n");
            return NULL;
        }

        // 保存初始状态
        superblock_save();
        bitmap_save();
        inode_save();
        disk_sync();

        printf("文件系统格式化完成！\n");
    } else {
        printf("加载现有文件系统...\n");

        // 加载超级块
        if (superblock_load() != 0) {
            printf("错误: 超级块加载失败\n");
            return NULL;
        }

        // 加载位图
        if (bitmap_load() != 0) {
            printf("错误: 位图加载失败\n");
            return NULL;
        }

        // 加载inode表
        if (inode_load() != 0) {
            printf("错误: inode表加载失败\n");
            return NULL;
        }

        printf("文件系统加载完成！\n");
    }

    g_fs.is_mounted = true;
    g_fs.is_dirty = false;

    printf("模块化EXT2文件系统初始化完成\n");
    printf("文件系统已挂载，可以使用标准文件操作\n");

    return NULL;
}

/**
 * FUSE清理
 */
static void fuse_cleanup(void *userdata) {
    (void) userdata;  // 忽略用户数据

    printf("正在卸载模块化EXT2文件系统...\n");

    if (g_fs.is_dirty) {
        printf("保存文件系统状态...\n");
        superblock_save();
        bitmap_save();
        inode_save();
        disk_sync();
    }

    // 清理资源
    if (g_fs.inode_table) {
        free(g_fs.inode_table);
        g_fs.inode_table = NULL;
    }

    if (g_fs.inode_bitmap) {
        free(g_fs.inode_bitmap);
        g_fs.inode_bitmap = NULL;
    }

    if (g_fs.block_bitmap) {
        free(g_fs.block_bitmap);
        g_fs.block_bitmap = NULL;
    }

    disk_cleanup();

    g_fs.is_mounted = false;
    printf("模块化EXT2文件系统已安全卸载\n");
}

/**
 * 其他FUSE操作函数
 */
static int fuse_opendir(const char *path, struct fuse_file_info *fi) {
    int inode_id = dir_resolve_path(path);
    if (inode_id == -1) {
        return -ENOENT;
    }

    if (!g_fs.inode_table[inode_id].is_directory) {
        return -ENOTDIR;
    }

    fi->fh = inode_id;
    return 0;
}

static int fuse_release(const char *path, struct fuse_file_info *fi) {
    (void) path; (void) fi;
    return 0;
}

static int fuse_releasedir(const char *path, struct fuse_file_info *fi) {
    (void) path; (void) fi;
    return 0;
}

static int fuse_fsync(const char *path, int isdatasync, struct fuse_file_info *fi) {
    (void) path; (void) isdatasync; (void) fi;

    if (g_fs.is_dirty) {
        superblock_save();
        bitmap_save();
        inode_save();
        disk_sync();
        g_fs.is_dirty = false;
    }

    return 0;
}

// ============================================================================
// FUSE操作结构体定义
// ============================================================================

struct fuse_operations fuse_operations = {
    .getattr    = fuse_getattr,
    .readdir    = fuse_readdir,
    .open       = fuse_open,
    .read       = fuse_read,
    .write      = fuse_write,
    .create     = fuse_create,
    .mkdir      = fuse_mkdir,
    .unlink     = fuse_unlink,
    .rmdir      = fuse_rmdir,
    .rename     = fuse_rename,
    .chmod      = fuse_chmod,
    .truncate   = fuse_truncate,
    .utimens    = fuse_utimens,
    .statfs     = fuse_statfs,
    .init       = fuse_init,
    .destroy    = fuse_cleanup,
    .opendir    = fuse_opendir,
    .release    = fuse_release,
    .releasedir = fuse_releasedir,
    .fsync      = fuse_fsync,
};
