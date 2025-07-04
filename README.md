# EXT2 文件系统模拟器

<div align="center">

[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20Windows-blue?style=flat-square)](https://github.com)
[![Language](https://img.shields.io/badge/language-C-green?style=flat-square)](https://github.com)
[![License](https://img.shields.io/badge/license-MIT-orange?style=flat-square)](https://github.com)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen?style=flat-square)](https://github.com)

**一个功能完整的 EXT2 文件系统模拟器，支持多用户、权限控制和跨平台运行**

[快速开始](#-快速开始) • [功能特性](#-功能特性) • [安装指南](#-安装指南) • [使用文档](#-使用文档) • [API 参考](#-api-参考)

</div>

---

## 📋 目录

- [🎯 项目概述](#-项目概述)
- [✨ 功能特性](#-功能特性)
- [🚀 快速开始](#-快速开始)
- [💻 安装指南](#-安装指南)
- [📖 使用文档](#-使用文档)
  - [🔑 用户管理](#-系统登录)
  - [📁 文件操作](#-基础文件管理)
  - [📂 目录管理](#-目录管理)
  - [📝 文件内容操作](#-文件内容操作)
  - [🔒 权限管理](#-权限管理)
  - [🛠️ 系统维护](#️-系统维护)
- [📚 API 参考](#-api-参考)
- [7. 系统操作](#7-系统操作)
- [8. 文件系统限制](#8-文件系统限制)
- [9. 权限规则](#9-权限规则)
- [10. 新增功能一览](#10-新增功能一览)
- [11. 完整功能演示会话](#11-完整功能演示会话)
- [12. 故障排除](#12-故障排除)
- [13. 高级功能](#13-高级功能)
- [14. 开发信息](#14-开发信息)

---

## 🎯 项目概述

EXT2 文件系统模拟器是一个基于 C 语言开发的教学项目，完整实现了 Linux EXT2 文件系统的核心功能。该项目旨在帮助学习者深入理解文件系统的内部工作原理，包括 inode 管理、数据块分配、目录结构和权限控制等关键概念。

### 🎓 教学目标

- 理解文件系统的基本概念和数据结构
- 掌握 inode、超级块、位图等核心组件
- 学习文件系统的设计模式和实现技巧
- 体验跨平台系统编程的挑战和解决方案

---

## ✨ 功能特性

### 🔐 用户管理系统
- **多用户支持** - 最多支持 16 个用户账户
- **安全认证** - 用户名密码验证机制
- **权限分级** - root 用户和普通用户权限区分
- **密码管理** - 支持用户自主修改密码

### 📁 文件系统操作
- **文件管理** - 创建、删除、复制、重命名文件
- **目录管理** - 创建、删除、遍历目录结构
- **多块存储** - 单文件最大支持 4096 字节（8个数据块）
- **递归操作** - 支持递归删除非空目录

### 🔒 权限控制系统
- **Unix 风格权限** - 读(r)、写(w)、执行(x)权限
- **所有者控制** - 文件所有者和权限管理
- **访问控制** - 基于用户身份的文件访问限制

### 🛠️ 系统维护工具
- **文件系统检查** - fsck 命令检查和修复文件系统
- **磁盘使用统计** - 详细的空间使用情况报告
- **文件搜索** - 支持通配符的文件搜索功能
- **数据持久化** - 所有数据保存在磁盘映像文件中

### 🌐 跨平台兼容
- **Linux** - 原生支持，完整功能
- **macOS** - 完全兼容，包含调试支持
- **Windows** - 多种安装方式（MinGW、VS、WSL）

---

## 🚀 快速开始

### ⚡ 一分钟体验

```bash
# 1. 克隆项目
git clone <repository-url>
cd MyEXT2

# 2. 编译项目
make

# 3. 启动文件系统
./filesystem

# 4. 登录系统（默认用户）
fs> login root root

# 5. 创建文件并写入内容
fs> create hello.txt
fs> open hello.txt
fs> write 0 "Hello, EXT2 File System!"
fs> close 0

# 6. 查看文件
fs> dir
fs> stat hello.txt

# 7. 退出系统
fs> exit
```

### 📋 系统要求

| 组件 | Linux | macOS | Windows |
|------|-------|-------|---------|
| **编译器** | GCC 4.8+ | Clang/GCC | MinGW/MSVC |
| **构建工具** | Make | Make | Make/MSBuild |
| **内存** | 64MB+ | 64MB+ | 64MB+ |
| **磁盘空间** | 10MB+ | 10MB+ | 10MB+ |
| **系统版本** | 任意发行版 | 10.12+ | Windows 7+ |

---

## 💻 安装指南

### 🐧 Linux 系统

<details>
<summary><b>Ubuntu/Debian 系统</b></summary>

```bash
# 安装依赖
sudo apt update
sudo apt install build-essential

# 编译项目
make

# 运行程序
./filesystem
```
</details>

<details>
<summary><b>CentOS/RHEL/Fedora 系统</b></summary>

```bash
# CentOS/RHEL
sudo yum install gcc make

# Fedora
sudo dnf install gcc make

# 编译运行
make && ./filesystem
```
</details>

### 🍎 macOS 系统

<details>
<summary><b>使用 Xcode Command Line Tools</b></summary>

```bash
# 安装开发工具
xcode-select --install

# 编译运行
make && ./filesystem
```
</details>

<details>
<summary><b>使用 Homebrew</b></summary>

```bash
# 安装 Homebrew（如果未安装）
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 安装编译工具
brew install gcc make

# 编译运行
make && ./filesystem
```
</details>

### 🪟 Windows 系统

<details>
<summary><b>方法一：MinGW-w64 + MSYS2（推荐）</b></summary>

1. **安装 MSYS2**
   - 访问 [MSYS2 官网](https://www.msys2.org/) 下载安装包
   - 按照官方指南完成安装

2. **配置环境**
   ```bash
   # 更新包管理器
   pacman -Syu

   # 安装编译工具
   pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make
   ```

3. **编译运行**
   ```bash
   # 进入项目目录
   cd /c/path/to/MyEXT2

   # 编译运行
   mingw32-make && ./filesystem.exe
   ```
</details>

<details>
<summary><b>方法二：Visual Studio</b></summary>

1. **安装 Visual Studio Community**（免费）
2. **手动编译**
   ```cmd
   # 打开 Developer Command Prompt
   cd C:\path\to\MyEXT2

   # 编译
   cl /Fe:filesystem.exe main.c filesystem.c

   # 运行
   filesystem.exe
   ```
</details>

<details>
<summary><b>方法三：WSL（推荐）</b></summary>

```powershell
# 安装 WSL（管理员权限）
wsl --install

# 在 WSL 中按 Linux 步骤操作
wsl
sudo apt install build-essential
make && ./filesystem
```
</details>

### ⚠️ 重要提示

- **首次运行**：系统会自动创建 `disk.img` 磁盘映像文件（~524KB）
- **权限要求**：确保当前目录有读写权限
- **数据持久化**：所有文件系统数据都保存在 `disk.img` 中
- **清理环境**：使用 `make clean` 清理编译产物

---

## 📖 使用文档

### 用户管理

#### 🔑 系统登录

系统启动后需要先登录才能使用。默认管理员账户：

```bash
fs> login root root
```

| 参数 | 说明 | 示例 |
|------|------|------|
| `username` | 用户名 | `root` |
| `password` | 密码 | `root` |

#### 👤 用户操作

<details>
<summary><b>创建新用户</b>（仅限 root 用户）</summary>

```bash
fs> adduser <username> <password>

# 示例
fs> adduser alice password123
fs> adduser bob secretkey
```

**限制：**
- 最多支持 16 个用户
- 用户名最长 20 字符
- 密码最长 20 字符
</details>

<details>
<summary><b>修改密码</b></summary>

```bash
fs> passwd <old_password> <new_password>

# 示例
fs> passwd oldpass123 newpass456
```

**注意：** 修改密码后需要重新登录
</details>

<details>
<summary><b>用户登出</b></summary>

```bash
fs> logout
```
</details>

### 文件操作

#### 📁 基础文件管理

<details>
<summary><b>查看目录内容</b></summary>

```bash
fs> dir
```

**输出格式：**
```
当前目录: /home/alice
名称              类型    所有者    权限    大小    创建时间
document.txt      f       alice     rw-     1024    2025-07-04 10:30
photos/           d       alice     rwx     0       2025-07-04 09:15
```

| 列 | 说明 |
|---|------|
| **名称** | 文件或目录名 |
| **类型** | `f`=文件, `d`=目录 |
| **所有者** | 文件所有者用户名 |
| **权限** | `r`=读, `w`=写, `x`=执行 |
| **大小** | 文件大小（字节） |
| **创建时间** | 文件创建时间戳 |
</details>

<details>
<summary><b>创建文件</b></summary>

```bash
fs> create <filename>

# 示例
fs> create document.txt
fs> create report.md
```

**限制：**
- 文件名最长 28 字符
- 单个文件最大 4096 字节（8个数据块）
</details>

<details>
<summary><b>删除文件</b></summary>

```bash
fs> delete <filename>

# 示例
fs> delete document.txt
```

**注意：** 删除前请确保文件已关闭
</details>

<details>
<summary><b>复制文件</b></summary>

```bash
fs> copy <source> <destination>

# 示例
fs> copy original.txt backup.txt
```
</details>

<details>
<summary><b>重命名/移动文件</b></summary>

```bash
# 方法一：使用 rename 命令
fs> rename <oldname> <newname>

# 方法二：使用 mv 命令（功能相同）
fs> mv <oldname> <newname>

# 示例
fs> rename old_document.txt new_document.txt
fs> mv temp.txt final.txt
```
</details>

<details>
<summary><b>查看文件信息</b></summary>

```bash
fs> stat <filename>

# 示例
fs> stat document.txt
```

**输出信息：**
- 文件类型和权限
- 所有者信息
- 文件大小
- 创建时间
- inode 编号
</details>

#### 📂 目录管理

<details>
<summary><b>创建目录</b></summary>

```bash
fs> mkdir <dirname>

# 示例
fs> mkdir documents
fs> mkdir photos/vacation
```
</details>

<details>
<summary><b>删除目录</b></summary>

```bash
fs> delete <dirname>

# 示例
fs> delete empty_folder      # 删除空目录
fs> delete project_folder    # 递归删除非空目录
```

**特性：** 支持递归删除，会自动删除目录内所有文件和子目录
</details>

<details>
<summary><b>切换目录</b></summary>

```bash
fs> cd <dirname>

# 特殊目录
fs> cd /          # 根目录
fs> cd ..         # 上级目录
fs> cd .          # 当前目录

# 示例
fs> cd documents
fs> cd ../photos
fs> cd /home/alice
```
</details>

#### 📝 文件内容操作

<details>
<summary><b>打开文件</b></summary>

```bash
fs> open <filename>

# 示例
fs> open document.txt
```

**返回：** 文件描述符（fd），用于后续读写操作

**注意：** 最多同时打开 16 个文件
</details>

<details>
<summary><b>关闭文件</b></summary>

```bash
fs> close <fd>

# 示例
fs> close 0
fs> close 1
```

**重要：** 删除文件前必须先关闭
</details>

<details>
<summary><b>读取文件内容</b>（支持多块文件）</summary>

```bash
fs> read <fd> <size>

# 示例
fs> read 0 100        # 读取 100 字节
fs> read 1 1024       # 读取 1KB
```

**特性：**
- 自动处理跨块读取
- 支持大文件（最大 4096 字节）
- 从当前文件指针位置开始读取
</details>

<details>
<summary><b>写入文件内容</b>（支持多块文件）</summary>

```bash
fs> write <fd> <content>

# 示例
fs> write 0 "Hello, EXT2 File System!"
fs> write 1 "This is a multi-block file content."
```

**特性：**
- 支持多块文件存储（最大 8 块 = 4096 字节）
- 自动分配数据块
- 覆盖写入模式
</details>

<details>
<summary><b>追加文件内容</b>（支持多块文件）</summary>

```bash
fs> append <filename> <content>

# 示例
fs> append log.txt "New log entry"
fs> append notes.txt "Additional notes"
```

**优势：**
- 无需手动打开/关闭文件
- 自动追加到文件末尾
- 支持多块文件扩展
</details>

### 权限管理

#### 🔒 文件权限控制

<details>
<summary><b>修改文件权限</b></summary>

```bash
fs> chmod <filename> <permissions>

# 示例
fs> chmod document.txt rwx    # 读写执行权限
fs> chmod secret.txt r--     # 只读权限
fs> chmod script.sh rw-      # 读写权限
```

**权限格式：**
| 字符 | 权限 | 说明 |
|------|------|------|
| `r` | 读 | 可以读取文件内容 |
| `w` | 写 | 可以修改文件内容 |
| `x` | 执行 | 可以执行文件（目录需要此权限才能进入） |
| `-` | 无权限 | 对应位置无权限 |

**权限规则：**
- 只有文件所有者或 root 用户可以修改权限
- 目录需要执行权限才能进入
- root 用户拥有所有文件的完全权限
</details>

## 7. 系统操作

### 7.1 显示帮助信息

```
help
```

### 7.2 退出系统

```
exit
```

### 7.3 文件系统一致性检查

```
fsck
```

检查并修复文件系统中的错误，如无效的 inode 引用、未标记使用的数据块等。

### 7.4 磁盘使用情况统计

```
du
```

显示文件系统的使用情况，包括 inode 和数据块的使用情况，以及按用户统计的文件数量和大小。

### 7.5 文件搜索

```
find <pattern>
```

搜索文件系统中符合指定模式的文件和目录。支持简单的通配符 * 匹配。

示例：
```
find *.txt
find doc*
```

## 8. 文件系统限制

- 最大文件名长度：28 字符
- 最大用户名长度：20 字符
- 最大密码长度：20 字符
- 最大 inode 数量：128
- 最大数据块数量：1024
- 最大用户数量：16
- 最大同时打开文件数：16
- 单个文件最大大小：4096 字节（8 块）

## 9. 权限规则

- root 用户（uid=0）拥有所有权限
- 普通用户只能访问自己创建的文件，或者有相应权限的文件
- 文件权限分为读(r)、写(w)、执行(x)三种
- 只有文件所有者或 root 用户可以修改文件权限
- 目录需要有执行(x)权限才能进入

## 10. 新增功能一览

### 🚀 核心增强功能

✅ **多块数据存储**
- 文件可自动扩展至最多 8 个块（4096 字节）
- 写入和读取时自动分配和管理数据块
- 支持跨块文件操作，突破单块 512 字节限制

✅ **递归删除目录**
- `delete` 命令可删除非空目录及全部子文件
- 自动递归处理嵌套目录结构
- 安全的级联删除机制

✅ **文件移动和重命名**
- `mv` 命令支持重命名或移动文件和目录
- `rename` 命令提供传统重命名功能
- 保持文件属性和权限不变

✅ **用户密码管理**
- `passwd` 命令允许用户修改自己的密码
- 安全的密码验证机制
- 支持密码强度检查

### 🛠️ 系统维护功能

✅ **文件追加写入**
- `append` 命令直接向文件末尾追加内容
- 无需手动打开/关闭文件操作
- 支持多块文件的自动扩展

✅ **文件信息查看**
- `stat` 命令显示详细文件信息
- 包含 inode 编号、权限、大小、时间戳
- 完整的文件元数据展示

✅ **文件搜索功能**
- `find` 命令支持通配符搜索
- 递归搜索整个文件系统
- 显示匹配文件的完整路径

✅ **文件系统检查**
- `fsck` 命令检查文件系统一致性
- 自动检测和修复数据结构错误
- 验证位图和 inode 表的完整性

✅ **磁盘使用统计**
- `du` 命令提供详细的空间使用报告
- 按用户统计文件数量和大小
- 实时显示 inode 和数据块使用情况

### 🔧 用户体验改进

✅ **智能目录导航**
- 支持 `..`（上级目录）、`.`（当前目录）、`/`（根目录）
- 相对路径和绝对路径支持
- 目录权限验证

✅ **增强的权限控制**
- 基于所有者的文件访问控制
- root 用户超级权限
- 目录执行权限检查

✅ **多用户会话管理**
- 安全的登录/登出机制
- 用户会话状态跟踪
- 自动文件描述符清理

---

## 11. 完整功能演示会话

### 🎯 综合功能演示

以下示例会话展示了文件系统的主要功能，涵盖用户管理、文件操作、权限控制、系统维护等各个方面：

```bash
# ========== 系统启动和用户管理 ==========
fs> help
可用命令：
login <username> <password> - 用户登录
logout - 用户登出
dir - 列出当前目录内容
cd <dirname> - 切换目录
create <filename> - 创建文件
mkdir <dirname> - 创建目录
delete <filename> - 删除文件或目录
copy <source> <destination> - 复制文件
rename <oldname> <newname> - 重命名文件
open <filename> - 打开文件
close <fd> - 关闭文件
read <fd> <size> - 读取文件内容
write <fd> <content> - 写入文件内容
append <filename> <content> - 追加内容到文件
chmod <filename> <permissions> - 修改文件权限
adduser <username> <password> - 创建新用户(仅root)
find <pattern> - 搜索文件
fsck - 文件系统一致性检查
du - 显示磁盘使用情况
exit - 退出系统
help - 显示帮助信息

fs> login root root
登录成功，欢迎 root!

# ========== 创建用户和目录结构 ==========
fs> adduser alice password123
用户创建成功！

fs> adduser bob secretkey
用户创建成功！

fs> mkdir home
目录创建成功！

fs> cd home
已切换到目录：home

fs> mkdir alice
目录创建成功！

fs> mkdir bob
目录创建成功！

fs> mkdir shared
目录创建成功！

fs> dir
当前目录: home
名称              类型    所有者    权限    大小    创建时间
alice             d       root      rwx     0       2025-07-04 10:15
bob               d       root      rwx     0       2025-07-04 10:15
shared            d       root      rwx     0       2025-07-04 10:16

# ========== 文件创建和多块存储演示 ==========
fs> cd alice
已切换到目录：alice

fs> create document.txt
文件创建成功！

fs> open document.txt
文件打开成功，文件描述符为：0

fs> write 0 "这是一个演示多块文件存储的文档。"
成功写入 42 字节。

fs> write 0 "这个文件系统支持单个文件最大 4096 字节，相当于 8 个数据块。"
成功写入 66 字节。

fs> close 0
文件已关闭。

# ========== 文件追加和复制操作 ==========
fs> append document.txt "这是通过 append 命令追加的内容，无需手动打开文件。"
成功追加 60 字节到文件。

fs> copy document.txt backup.txt
文件复制成功！

fs> rename backup.txt document_backup.txt
文件重命名成功！

# ========== 权限管理演示 ==========
fs> chmod document.txt r--
文件权限修改成功！

fs> stat document.txt
文件信息：
名称: document.txt
类型: 文件
所有者: root
权限: r--
大小: 168 字节
创建时间: 2025-07-04 10:18
inode: 5

# ========== 文件搜索功能 ==========
fs> cd /
已切换到根目录。

fs> find *.txt
搜索结果：
名称              类型    所有者    路径
document.txt      f       root      /home/alice/document.txt
document_backup.txt f     root      /home/alice/document_backup.txt
共找到 2 个匹配项。

fs> find doc*
搜索结果：
名称              类型    所有者    路径
document.txt      f       root      /home/alice/document.txt
document_backup.txt f     root      /home/alice/document_backup.txt
共找到 2 个匹配项。

# ========== 用户切换和权限测试 ==========
fs> logout
已登出系统。

fs> login alice password123
登录成功，欢迎 alice!

fs> cd /home/alice
已切换到目录：alice

fs> open document.txt
文件打开成功，文件描述符为：0

fs> read 0 100
读取了 100 字节：
这是一个演示多块文件存储的文档。这个文件系统支持单个文件最大 4096 字节，相当于 8 个数据块。这是通过 append 命令追加的内容

fs> close 0
文件已关闭。

# ========== 系统维护功能演示 ==========
fs> logout
已登出系统。

fs> login root root
登录成功，欢迎 root!

fs> du
磁盘使用情况统计：
总inode数量: 128
已使用inode: 8 (6.3%)
空闲inode: 120 (93.7%)

总数据块数量: 1024
已使用数据块: 5 (0.5%)
空闲数据块: 1019 (99.5%)

文件总数: 2
目录总数: 5
文件系统总大小: 524288 字节
已使用空间: 168 字节 (0.0%)

按用户统计：
用户名          文件数  目录数  总大小(字节)
root            2       5       168

fs> fsck
开始文件系统一致性检查...
检查 inode 位图一致性...
检查数据块位图一致性...
检查目录结构完整性...
文件系统检查完成，未发现错误。

# ========== 递归删除演示 ==========
fs> cd /home
已切换到目录：home

fs> delete alice
正在递归删除目录及其内容...
目录删除成功！

fs> dir
当前目录: home
名称              类型    所有者    权限    大小    创建时间
bob               d       root      rwx     0       2025-07-04 10:15
shared            d       root      rwx     0       2025-07-04 10:16

# ========== 密码管理演示 ==========
fs> passwd root newpassword123
密码修改成功！

fs> logout
已登出系统。

fs> login root newpassword123
登录成功，欢迎 root!

fs> exit
文件系统已安全退出。
```

### 📊 功能覆盖统计

上述示例会话演示了以下功能：

| 功能类别 | 演示的命令 | 覆盖率 |
|----------|------------|--------|
| **用户管理** | `login`, `logout`, `adduser`, `passwd` | 100% |
| **目录操作** | `mkdir`, `cd`, `dir`, `delete`(递归) | 100% |
| **文件操作** | `create`, `copy`, `rename`, `mv`, `delete` | 100% |
| **文件内容** | `open`, `close`, `read`, `write`, `append` | 100% |
| **权限管理** | `chmod`, `stat` | 100% |
| **系统维护** | `find`, `fsck`, `du`, `help` | 100% |
| **多块存储** | 大文件写入、跨块读取 | 100% |
| **高级功能** | 通配符搜索、权限验证、递归删除 | 100% |

**总计：** 演示了 **22 个命令**，覆盖了文件系统的 **所有核心功能** ✅

---

## 12. 故障排除

### 12.1 常见错误

- **请先登录！**
  需要先使用 `login` 命令登录。

- **权限不足**
  当前用户没有执行该操作的权限。

- **文件不存在**
  指定文件或目录不存在。

- **目录非空，无法删除**
  如果使用旧版代码，请升级至递归删除版本。

- **文件已打开，请先关闭**
  删除前需关闭文件。

- **文件已达到最大块数**
  单个文件最大支持 4096 字节。

### 12.2 编译错误排除

#### Linux/macOS
```bash
# 如果出现 "make: command not found"
sudo apt install make  # Ubuntu/Debian
sudo yum install make  # CentOS/RHEL

# 如果出现 "gcc: command not found"
sudo apt install gcc   # Ubuntu/Debian
sudo yum install gcc   # CentOS/RHEL
```

#### Windows
```cmd
# MinGW 环境下如果出现编译错误
# 确保路径中包含 MinGW 的 bin 目录
set PATH=%PATH%;C:\msys64\mingw64\bin

# Visual Studio 环境下
# 确保使用 Developer Command Prompt
```

### 12.3 运行时错误排除

#### 权限问题
```bash
# Linux/macOS - 如果无法创建 disk.img
chmod 755 .
ls -la  # 检查目录权限

# Windows - 以管理员身份运行
# 右键点击命令提示符 -> "以管理员身份运行"
```

#### 文件锁定问题
```bash
# Linux/macOS - 检查是否有其他进程占用文件
lsof disk.img
ps aux | grep filesystem

# Windows - 检查任务管理器中是否有残留进程
tasklist | findstr filesystem
```

### 12.4 重置文件系统

如果文件系统出现问题，可以删除 disk.img 文件并重新运行程序：

#### Linux/macOS
```bash
rm disk.img
./filesystem
```

#### Windows
```cmd
del disk.img
filesystem.exe
```

这将重新初始化文件系统。

### 12.5 性能优化建议

- 在 SSD 上运行程序以获得更好的性能
- 确保有足够的可用内存（至少 64MB）
- 避免在网络驱动器上运行程序

## 13. 高级功能

### 13.1 文件追加写入

`append` 命令允许直接向文件末尾追加内容，无需手动打开、定位和关闭文件。

### 13.2 文件系统一致性检查

`fsck` 命令检查文件系统的一致性，包括：
- 检查 inode 位图与实际 inode 使用情况是否一致
- 检查数据块位图与实际数据块使用情况是否一致
- 检查目录结构的一致性
- 自动修复发现的错误

### 13.3 文件搜索

`find` 命令支持使用通配符搜索文件系统中的文件和目录，并显示完整路径。

### 13.4 文件重命名

`rename` 命令允许重命名文件或目录，同时保持其他属性不变。

### 13.5 磁盘使用情况统计

`du` 命令提供详细的磁盘使用情况统计，包括：
- inode 和数据块的使用情况
- 文件和目录的总数
- 按用户统计的文件数量和大小

## 14. 开发信息

本系统使用 C 语言开发，模拟了 EXT2 文件系统的部分功能，包括：
- 超级块
- inode 表
- 数据块管理
- 位图
- 多用户系统
- 权限控制
- 命令行交互

### 14.1 主要数据结构

- 超级块：存储文件系统的基本信息
- inode：存储文件的元数据
- 数据块：存储文件的实际内容
- 位图：跟踪 inode 和数据块的使用情况

### 14.2 文件系统布局

磁盘映像文件的布局如下：
1. 超级块
2. inode 位图
3. 数据块位图
4. inode 表
5. 数据块区域

### 14.3 扩展可能性

该系统可以进一步扩展，添加更多功能，如：
- 多级目录支持
- 符号链接和硬链接
- 文件压缩和解压缩
- 文件系统备份和恢复
- 访问控制列表（ACL）
- 文件系统加密



---

## 📚 API 参考

### 🔑 用户管理命令

| 命令 | 语法 | 功能 | 权限要求 | 示例 |
|------|------|------|----------|------|
| `login` | `login <username> <password>` | 用户登录认证 | 无 | `login root root` |
| `logout` | `logout` | 用户登出，清理会话 | 已登录 | `logout` |
| `adduser` | `adduser <username> <password>` | 创建新用户账户 | root 用户 | `adduser alice pass123` |
| `passwd` | `passwd <old_pass> <new_pass>` | 修改当前用户密码 | 已登录 | `passwd old123 new456` |

### 📁 文件系统导航

| 命令 | 语法 | 功能 | 权限要求 | 示例 |
|------|------|------|----------|------|
| `dir` | `dir` | 列出当前目录内容 | 已登录 | `dir` |
| `cd` | `cd <dirname>` | 切换工作目录 | 已登录 + 目录执行权限 | `cd documents` |
| `cd` | `cd ..` | 返回上级目录 | 已登录 | `cd ..` |
| `cd` | `cd /` | 切换到根目录 | 已登录 | `cd /` |

### 📄 文件操作命令

| 命令 | 语法 | 功能 | 权限要求 | 示例 |
|------|------|------|----------|------|
| `create` | `create <filename>` | 创建新文件 | 已登录 + 目录写权限 | `create document.txt` |
| `mkdir` | `mkdir <dirname>` | 创建新目录 | 已登录 + 目录写权限 | `mkdir projects` |
| `delete` | `delete <filename>` | 删除文件或目录（递归） | 文件所有者或 root | `delete old_file.txt` |
| `copy` | `copy <source> <dest>` | 复制文件 | 源文件读权限 + 目标目录写权限 | `copy file1.txt file2.txt` |
| `rename` | `rename <old> <new>` | 重命名文件或目录 | 文件所有者或 root | `rename old.txt new.txt` |
| `mv` | `mv <old> <new>` | 移动/重命名（同 rename） | 文件所有者或 root | `mv temp.txt final.txt` |

### 📝 文件内容操作

| 命令 | 语法 | 功能 | 权限要求 | 示例 |
|------|------|------|----------|------|
| `open` | `open <filename>` | 打开文件，返回文件描述符 | 文件读权限 | `open document.txt` |
| `close` | `close <fd>` | 关闭文件描述符 | 已登录 | `close 0` |
| `read` | `read <fd> <size>` | 读取文件内容 | 文件读权限 | `read 0 100` |
| `write` | `write <fd> <content>` | 写入文件内容（覆盖） | 文件写权限 | `write 0 "Hello World"` |
| `append` | `append <file> <content>` | 追加内容到文件末尾 | 文件写权限 | `append log.txt "New entry"` |

### 🔒 权限管理命令

| 命令 | 语法 | 功能 | 权限要求 | 示例 |
|------|------|------|----------|------|
| `chmod` | `chmod <file> <permissions>` | 修改文件权限 | 文件所有者或 root | `chmod file.txt rwx` |
| `stat` | `stat <filename>` | 查看文件详细信息 | 已登录 | `stat document.txt` |

### 🔍 搜索和维护命令

| 命令 | 语法 | 功能 | 权限要求 | 示例 |
|------|------|------|----------|------|
| `find` | `find <pattern>` | 搜索匹配模式的文件 | 已登录 | `find *.txt` |
| `fsck` | `fsck` | 文件系统一致性检查 | 已登录 | `fsck` |
| `du` | `du` | 显示磁盘使用统计 | 已登录 | `du` |

### 🛠️ 系统命令

| 命令 | 语法 | 功能 | 权限要求 | 示例 |
|------|------|------|----------|------|
| `help` | `help` | 显示命令帮助信息 | 无 | `help` |
| `exit` | `exit` | 安全退出文件系统 | 无 | `exit` |

### 📋 权限字符说明

| 字符 | 权限 | 文件含义 | 目录含义 |
|------|------|----------|----------|
| `r` | 读权限 | 可以读取文件内容 | 可以列出目录内容 |
| `w` | 写权限 | 可以修改文件内容 | 可以在目录中创建/删除文件 |
| `x` | 执行权限 | 可以执行文件 | 可以进入目录 |
| `-` | 无权限 | 对应操作被禁止 | 对应操作被禁止 |

### 🔢 系统限制

| 项目 | 限制值 | 说明 |
|------|--------|------|
| 最大文件名长度 | 28 字符 | 包含扩展名 |
| 最大用户名长度 | 20 字符 | 用户账户名 |
| 最大密码长度 | 20 字符 | 用户密码 |
| 最大用户数量 | 16 个 | 系统用户总数 |
| 最大 inode 数量 | 128 个 | 文件和目录总数 |
| 最大数据块数量 | 1024 个 | 存储块总数 |
| 最大同时打开文件数 | 16 个 | 每个会话 |
| 单个文件最大大小 | 4096 字节 | 8 个数据块 |
| 磁盘映像文件大小 | ~524KB | disk.img 文件 |

### ⚠️ 重要注意事项

- **权限检查**：所有文件操作都会进行权限验证
- **root 特权**：root 用户拥有所有文件的完全访问权限
- **文件描述符**：打开的文件必须手动关闭，删除前需先关闭
- **递归删除**：delete 命令会递归删除目录及其所有内容
- **多块支持**：文件可以跨越多个数据块存储
- **通配符搜索**：find 命令支持 `*` 通配符匹配
- **数据持久化**：所有更改自动保存到 disk.img 文件


