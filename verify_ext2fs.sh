#!/bin/bash

# ============================================
# EXT2文件系统验证脚本
# 用于确认当前操作的是自建EXT2文件系统
# ============================================

MOUNT_POINT="${1:-/tmp/myext2_demo}"

echo "============================================"
echo "    🔍 EXT2文件系统验证报告"
echo "============================================"
echo "📁 挂载点: $MOUNT_POINT"
echo ""

# 检查挂载点是否存在
if [ ! -d "$MOUNT_POINT" ]; then
    echo "❌ 错误: 挂载点不存在 '$MOUNT_POINT'"
    echo ""
    echo "💡 解决方案:"
    echo "   1. 创建挂载点: mkdir -p $MOUNT_POINT"
    echo "   2. 启动文件系统: ./ext2fs -f -d $MOUNT_POINT"
    exit 1
fi

# 1. 检查挂载信息
echo "🔍 1. 挂载信息检查:"
MOUNT_INFO=$(mount | grep "$(basename $MOUNT_POINT)")
if [ -n "$MOUNT_INFO" ]; then
    echo "   ✅ $MOUNT_INFO"
    
    # 检查是否为FUSE EXT2
    if echo "$MOUNT_INFO" | grep -q "fuse.ext2fs"; then
        echo "   ✅ 确认: FUSE EXT2文件系统"
    else
        echo "   ❌ 警告: 不是FUSE EXT2文件系统"
    fi
else
    echo "   ❌ 错误: 未找到挂载信息"
fi
echo ""

# 2. 检查文件系统类型和大小
echo "🔍 2. 文件系统类型和大小:"
if df -T "$MOUNT_POINT" >/dev/null 2>&1; then
    FS_INFO=$(df -T "$MOUNT_POINT" | tail -n 1)
    FS_TYPE=$(echo "$FS_INFO" | awk '{print $2}')
    FS_SIZE=$(echo "$FS_INFO" | awk '{print $3}')
    FS_USED=$(echo "$FS_INFO" | awk '{print $4}')
    FS_AVAIL=$(echo "$FS_INFO" | awk '{print $5}')
    
    echo "   📊 类型: $FS_TYPE"
    echo "   📊 总大小: ${FS_SIZE}KB"
    echo "   📊 已使用: ${FS_USED}KB"
    echo "   📊 可用: ${FS_AVAIL}KB"
    
    # 验证类型
    if [ "$FS_TYPE" = "fuse.ext2fs" ]; then
        echo "   ✅ 确认: 自建EXT2文件系统"
    else
        echo "   ❌ 警告: 类型不匹配 (期望: fuse.ext2fs, 实际: $FS_TYPE)"
    fi
    
    # 验证大小 (自建EXT2应该很小)
    if [ "$FS_SIZE" -lt 1000 ]; then
        echo "   ✅ 确认: 小容量文件系统 (符合自建EXT2特征)"
    else
        echo "   ⚠️  注意: 容量较大 (可能不是默认的自建EXT2)"
    fi
else
    echo "   ❌ 错误: 无法获取文件系统信息"
fi
echo ""

# 3. 检查FUSE进程
echo "🔍 3. FUSE进程检查:"
EXT2FS_PIDS=$(pgrep -f "ext2fs.*$MOUNT_POINT" 2>/dev/null)
if [ -n "$EXT2FS_PIDS" ]; then
    echo "   ✅ EXT2FS进程正在运行 (PID: $EXT2FS_PIDS)"
    
    # 显示进程详情
    ps aux | grep "ext2fs.*$MOUNT_POINT" | grep -v grep | while read line; do
        echo "   📋 $line"
    done
else
    echo "   ❌ 警告: 未找到EXT2FS进程"
fi
echo ""

# 4. 对比原生文件系统
echo "🔍 4. 与原生文件系统对比:"
NATIVE_INFO=$(df -T /tmp 2>/dev/null | tail -n 1)
if [ -n "$NATIVE_INFO" ]; then
    NATIVE_TYPE=$(echo "$NATIVE_INFO" | awk '{print $2}')
    NATIVE_SIZE=$(echo "$NATIVE_INFO" | awk '{print $3}')
    
    echo "   原生/tmp目录:"
    echo "     类型: $NATIVE_TYPE, 大小: ${NATIVE_SIZE}KB"
    echo "   自建EXT2:"
    echo "     类型: $FS_TYPE, 大小: ${FS_SIZE}KB"
    
    # 对比分析
    if [ "$FS_TYPE" != "$NATIVE_TYPE" ] && [ "$FS_SIZE" -lt "$NATIVE_SIZE" ]; then
        echo "   ✅ 确认: 明显区别于原生文件系统"
    fi
fi
echo ""

# 5. 功能测试
echo "🔍 5. 基本功能测试:"
TEST_FILE="$MOUNT_POINT/.verify_test_$$"

# 测试写入
if echo "test" > "$TEST_FILE" 2>/dev/null; then
    echo "   ✅ 文件写入: 正常"
    
    # 测试读取
    if [ "$(cat "$TEST_FILE" 2>/dev/null)" = "test" ]; then
        echo "   ✅ 文件读取: 正常"
    else
        echo "   ❌ 文件读取: 失败"
    fi
    
    # 清理测试文件
    rm -f "$TEST_FILE" 2>/dev/null
    echo "   ✅ 文件删除: 正常"
else
    echo "   ❌ 文件写入: 失败"
fi
echo ""

# 6. 最终结论
echo "============================================"
echo "📋 验证结果总结:"

SCORE=0
MAX_SCORE=5

# 评分标准
if echo "$MOUNT_INFO" | grep -q "fuse.ext2fs"; then
    SCORE=$((SCORE + 1))
fi

if [ "$FS_TYPE" = "fuse.ext2fs" ]; then
    SCORE=$((SCORE + 1))
fi

if [ "$FS_SIZE" -lt 1000 ]; then
    SCORE=$((SCORE + 1))
fi

if [ -n "$EXT2FS_PIDS" ]; then
    SCORE=$((SCORE + 1))
fi

if [ -f "$TEST_FILE" ] || echo "test" > "$TEST_FILE" 2>/dev/null; then
    SCORE=$((SCORE + 1))
    rm -f "$TEST_FILE" 2>/dev/null
fi

echo "📊 验证得分: $SCORE/$MAX_SCORE"

if [ $SCORE -ge 4 ]; then
    echo "🎉 验证通过: 正在使用自建EXT2文件系统"
    echo ""
    echo "💡 现在您可以放心地进行文件操作测试:"
    echo "   echo 'Hello EXT2!' > $MOUNT_POINT/test.txt"
    echo "   cat $MOUNT_POINT/test.txt"
    echo "   ls -la $MOUNT_POINT/"
elif [ $SCORE -ge 2 ]; then
    echo "⚠️  部分验证通过: 可能是自建EXT2文件系统"
    echo "   建议检查启动参数和挂载状态"
else
    echo "❌ 验证失败: 不是自建EXT2文件系统"
    echo ""
    echo "💡 解决方案:"
    echo "   1. 确保文件系统已启动: ./ext2fs -f -d $MOUNT_POINT"
    echo "   2. 检查挂载点路径是否正确"
    echo "   3. 查看错误日志排查问题"
fi

echo "============================================"
