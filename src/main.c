/*
 * ============================================================================
 * 文件名: src/main.c
 * 描述: 模块化EXT2文件系统主程序
 * 功能: 命令行处理和FUSE启动
 * ============================================================================
 */

#define FUSE_USE_VERSION 30
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <fuse.h>
#include "../include/ext2fs.h"
#include "fuse/operations.h"

// ============================================================================
// 程序信息
// ============================================================================
#define PROGRAM_NAME "ext2fs"

// ============================================================================
// 帮助和版本信息
// ============================================================================

static void show_usage(const char *progname) {
    printf("用法: %s [选项] <挂载点>\n", progname);
    printf("\n");
    printf("模块化EXT2文件系统 v%s\n", EXT2FS_VERSION);
    printf("基于FUSE技术的模块化EXT2文件系统实现\n");
    printf("\n");
    printf("选项:\n");
    printf("  -h, --help        显示此帮助信息\n");
    printf("  -V, --version     显示版本信息\n");
    printf("  -f                前台运行(不后台化)\n");
    printf("  -d                启用调试模式\n");
    printf("  -s                单线程模式\n");
    printf("  -o opt[,opt...]   挂载选项\n");
    printf("\n");
    printf("挂载选项:\n");
    printf("  ro                只读挂载\n");
    printf("  allow_other       允许其他用户访问\n");
    printf("  default_permissions 启用权限检查\n");
    printf("\n");
    printf("示例:\n");
    printf("  %s /tmp/myfs                    # 基本挂载\n", progname);
    printf("  %s -f -d /tmp/myfs              # 前台调试模式\n", progname);
    printf("  %s -o allow_other /tmp/myfs     # 允许其他用户访问\n", progname);
    printf("\n");
    printf("模块化架构:\n");
    printf("  src/fs/          文件系统核心模块\n");
    printf("  src/core/        底层核心模块\n");
    printf("  src/fuse/        FUSE接口模块\n");
    printf("  include/         公共头文件\n");
    printf("\n");
    printf("卸载:\n");
    printf("  fusermount -u /tmp/myfs         # 卸载文件系统\n");
    printf("\n");
}

static void show_version(void) {
    printf("模块化EXT2文件系统 v%s\n", EXT2FS_VERSION);
    printf("基于FUSE %d.%d的模块化EXT2文件系统实现\n", 
           FUSE_MAJOR_VERSION, FUSE_MINOR_VERSION);
    printf("编译时间: %s %s\n", __DATE__, __TIME__);
    printf("\n");
    printf("架构特性:\n");
    printf("- 模块化设计，职责分离清晰\n");
    printf("- 超级块、inode、数据块独立管理\n");
    printf("- 标准的文件系统项目结构\n");
    printf("- 完整的POSIX兼容接口\n");
    printf("- 数据持久化存储\n");
    printf("- 高效的位图管理\n");
    printf("- 可扩展的模块架构\n");
    printf("\n");
    printf("模块列表:\n");
    printf("- superblock: 超级块管理\n");
    printf("- inode:      索引节点管理\n");
    printf("- block:      数据块管理\n");
    printf("- directory:  目录操作\n");
    printf("- file:       文件操作\n");
    printf("- disk:       磁盘I/O\n");
    printf("- bitmap:     位图管理\n");
    printf("- operations: FUSE接口\n");
}

static void show_banner(void) {
    printf("============================================\n");
    printf("    模块化EXT2文件系统 v%s\n", EXT2FS_VERSION);
    printf("    基于FUSE的模块化文件系统实现\n");
    printf("============================================\n");
}

// ============================================================================
// 主函数
// ============================================================================

int main(int argc, char *argv[]) {
    show_banner();
    
    // 检查参数
    if (argc < 2) {
        show_usage(argv[0]);
        return 1;
    }
    
    // 处理帮助和版本选项
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            show_usage(argv[0]);
            return 0;
        }
        if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--version") == 0) {
            show_version();
            return 0;
        }
    }
    
    printf("正在启动模块化文件系统...\n");
    printf("磁盘镜像: %s\n", DISK_IMAGE);
    printf("挂载点: %s\n", argv[argc-1]);
    printf("\n");
    printf("模块化架构:\n");
    printf("- 文件系统核心: src/fs/\n");
    printf("- 底层核心: src/core/\n");
    printf("- FUSE接口: src/fuse/\n");
    printf("\n");
    printf("提示:\n");
    printf("- 使用标准文件命令操作 (ls, cp, mkdir, etc.)\n");
    printf("- 使用 'fusermount -u %s' 卸载\n", argv[argc-1]);
    printf("- 按 Ctrl+C 停止 (如果使用 -f 选项)\n");
    printf("\n");
    
    // 启动FUSE
    int result = fuse_main(argc, argv, &fuse_operations, NULL);
    
    if (result != 0) {
        printf("FUSE启动失败，错误码: %d\n", result);
        printf("请检查:\n");
        printf("1. 挂载点目录是否存在且为空\n");
        printf("2. 是否有足够的权限\n");
        printf("3. FUSE是否正确安装\n");
        printf("4. 是否已有其他文件系统挂载在该点\n");
        printf("\n");
        printf("调试建议:\n");
        printf("- 使用 -f -d 选项启用前台调试模式\n");
        printf("- 检查 dmesg 输出查看内核消息\n");
        printf("- 确认 /dev/fuse 设备存在且可访问\n");
    }
    
    return result;
}
