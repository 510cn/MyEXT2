# ============================================================================
# Makefile for Modular EXT2 Filesystem
# ============================================================================

# 编译器和标志
CC = gcc
CFLAGS = -Wall -Wextra -g -std=c99 -D_FILE_OFFSET_BITS=64
FUSE_CFLAGS = $(shell pkg-config fuse --cflags)
FUSE_LIBS = $(shell pkg-config fuse --libs)

# 项目信息
PROJECT_NAME = ext2fs
VERSION = 4.0
TARGET = $(PROJECT_NAME)

# 目录结构
SRCDIR = src
INCDIR = include
OBJDIR = obj

# 源文件分类
FS_SOURCES = $(SRCDIR)/fs/superblock.c $(SRCDIR)/fs/inode.c $(SRCDIR)/fs/block.c \
             $(SRCDIR)/fs/directory.c $(SRCDIR)/fs/file.c
CORE_SOURCES = $(SRCDIR)/core/disk.c $(SRCDIR)/core/bitmap.c
FUSE_SOURCES = $(SRCDIR)/fuse/operations.c
MAIN_SOURCES = $(SRCDIR)/main.c

# 所有源文件
SOURCES = $(FS_SOURCES) $(CORE_SOURCES) $(FUSE_SOURCES) $(MAIN_SOURCES)

# 对象文件
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# 头文件
HEADERS = $(INCDIR)/ext2fs.h

# 包含路径
INCLUDES = -I$(INCDIR)

# ============================================================================
# 构建目标
# ============================================================================

.PHONY: all clean install uninstall test help version check-deps dirs

# 默认目标
all: check-deps dirs $(TARGET)

# 创建目录
dirs:
	@mkdir -p $(OBJDIR)/fs $(OBJDIR)/core $(OBJDIR)/fuse

# 主程序
$(TARGET): $(OBJECTS)
	@echo "正在链接 $(TARGET)..."
	$(CC) $(CFLAGS) $(FUSE_CFLAGS) -o $@ $^ $(FUSE_LIBS)
	@echo "构建完成: $(TARGET) v$(VERSION)"

# 编译规则
$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HEADERS)
	@echo "正在编译 $<..."
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(FUSE_CFLAGS) $(INCLUDES) -c $< -o $@

# ============================================================================
# 依赖检查
# ============================================================================

check-deps:
	@echo "检查构建依赖..."
	@which pkg-config > /dev/null || (echo "错误: 需要安装 pkg-config" && exit 1)
	@pkg-config --exists fuse || (echo "错误: 需要安装 libfuse-dev 或 fuse-devel" && exit 1)
	@echo "依赖检查通过"

# ============================================================================
# 模块化构建
# ============================================================================

# 只构建文件系统核心模块
fs-core: dirs $(OBJDIR)/fs/superblock.o $(OBJDIR)/fs/inode.o $(OBJDIR)/fs/block.o \
         $(OBJDIR)/fs/directory.o $(OBJDIR)/fs/file.o $(OBJDIR)/core/disk.o $(OBJDIR)/core/bitmap.o
	@echo "文件系统核心模块构建完成"

# 只构建FUSE接口模块
fuse-interface: dirs $(OBJDIR)/fuse/operations.o
	@echo "FUSE接口模块构建完成"

# 构建测试版本 (带调试信息)
debug: CFLAGS += -DDEBUG -O0 -ggdb
debug: clean all

# 构建发布版本 (优化)
release: CFLAGS += -O2 -DNDEBUG
release: clean all

# ============================================================================
# 测试功能
# ============================================================================

# 创建测试环境
test-setup:
	@echo "创建测试环境..."
	@mkdir -p /tmp/ext2fs_modular_test
	@echo "测试挂载点: /tmp/ext2fs_modular_test"

# 前台测试挂载
test-mount: $(TARGET) test-setup
	@echo "前台挂载模块化文件系统 (按 Ctrl+C 停止)..."
	./$(TARGET) -f -d /tmp/ext2fs_modular_test

# 后台测试挂载
test-mount-bg: $(TARGET) test-setup
	@echo "后台挂载模块化文件系统..."
	./$(TARGET) /tmp/ext2fs_modular_test
	@echo "文件系统已挂载到 /tmp/ext2fs_modular_test"
	@echo "使用 'make test-umount' 卸载"

# 卸载测试文件系统
test-umount:
	@echo "卸载测试文件系统..."
	@fusermount -u /tmp/ext2fs_modular_test 2>/dev/null || true
	@echo "文件系统已卸载"

# 清理测试环境
test-clean: test-umount
	@echo "清理测试环境..."
	@rmdir /tmp/ext2fs_modular_test 2>/dev/null || true
	@echo "测试环境已清理"

# 运行模块化测试
test-modular: test-mount-bg
	@echo "运行模块化文件系统测试..."
	@sleep 2
	
	@echo "1. 测试超级块功能..."
	@echo "Hello Modular EXT2!" > /tmp/ext2fs_modular_test/test.txt || echo "创建文件失败"
	
	@echo "2. 测试inode管理..."
	@mkdir /tmp/ext2fs_modular_test/testdir 2>/dev/null || echo "创建目录失败"
	
	@echo "3. 测试数据块管理..."
	@cat /tmp/ext2fs_modular_test/test.txt 2>/dev/null || echo "读取文件失败"
	
	@echo "4. 测试位图管理..."
	@ls -la /tmp/ext2fs_modular_test/ 2>/dev/null || echo "列出目录失败"
	
	@echo "5. 测试磁盘I/O..."
	@df -h /tmp/ext2fs_modular_test/ 2>/dev/null || echo "获取文件系统信息失败"
	
	@echo "模块化测试完成! 使用 'make test-clean' 清理环境"

# ============================================================================
# 清理和维护
# ============================================================================

clean:
	@echo "清理构建文件..."
	@rm -rf $(OBJDIR) $(TARGET)
	@echo "清理完成"

distclean: clean test-clean
	@echo "完全清理..."
	@rm -f disk.img *.log
	@echo "完全清理完成"

# ============================================================================
# 代码质量
# ============================================================================

# 代码统计
stats:
	@echo "模块化代码统计信息:"
	@echo "源文件数量:"
	@find $(SRCDIR) -name "*.c" | wc -l
	@echo "头文件数量:"
	@find $(SRCDIR) $(INCDIR) -name "*.h" | wc -l
	@echo "总代码行数:"
	@find $(SRCDIR) $(INCDIR) -name "*.c" -o -name "*.h" | xargs wc -l | tail -1
	@echo ""
	@echo "模块分布:"
	@echo "文件系统核心模块:"
	@find $(SRCDIR)/fs -name "*.c" -o -name "*.h" | xargs wc -l | tail -1
	@echo "底层核心模块:"
	@find $(SRCDIR)/core -name "*.c" -o -name "*.h" | xargs wc -l | tail -1
	@echo "FUSE接口模块:"
	@find $(SRCDIR)/fuse -name "*.c" -o -name "*.h" | xargs wc -l | tail -1

# ============================================================================
# 信息和帮助
# ============================================================================

version:
	@echo "$(PROJECT_NAME) version $(VERSION) (模块化架构)"

help:
	@echo "模块化EXT2文件系统构建系统"
	@echo ""
	@echo "使用方法: make [目标]"
	@echo ""
	@echo "构建目标:"
	@echo "  all          构建完整项目 (默认)"
	@echo "  fs-core      只构建文件系统核心模块"
	@echo "  fuse-interface 只构建FUSE接口模块"
	@echo "  debug        构建调试版本"
	@echo "  release      构建发布版本"
	@echo "  clean        清理构建文件"
	@echo "  distclean    完全清理"
	@echo ""
	@echo "测试目标:"
	@echo "  test-setup   创建测试环境"
	@echo "  test-mount   前台挂载测试"
	@echo "  test-mount-bg 后台挂载测试"
	@echo "  test-umount  卸载测试文件系统"
	@echo "  test-clean   清理测试环境"
	@echo "  test-modular 运行模块化测试"
	@echo ""
	@echo "代码质量:"
	@echo "  stats        代码统计"
	@echo ""
	@echo "信息目标:"
	@echo "  version      显示版本信息"
	@echo "  help         显示此帮助"
	@echo "  check-deps   检查构建依赖"
	@echo ""
	@echo "模块化架构:"
	@echo "  $(SRCDIR)/fs/        文件系统核心模块"
	@echo "  $(SRCDIR)/core/      底层核心模块"
	@echo "  $(SRCDIR)/fuse/      FUSE接口模块"
	@echo "  $(INCDIR)/           公共头文件"
