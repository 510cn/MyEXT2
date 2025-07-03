# Linux 二级文件系统模拟器

## 1. 系统概述

Linux 二级文件系统模拟器是一个简单的文件系统实现，模拟了 Linux 文件系统的基本功能。该系统支持用户管理、文件和目录操作、权限控制、多块文件存储等功能。

---

## 2. 编译与运行

### 2.1 编译

在项目目录下执行以下命令编译系统：

```bash
make
```

将生成可执行文件 `filesystem`。

### 2.2 运行

首次运行时，系统会自动创建一个名为 `disk.img` 的磁盘映像文件，并初始化文件系统：

```bash
./filesystem
```

---

## 3. 用户管理

### 3.1 默认用户

系统初始化时会创建一个默认的 root 用户：
- 用户名：root
- 密码：root

### 3.2 用户登录

```
login <username> <password>
```

### 3.3 用户登出

```
logout
```

### 3.4 创建新用户（仅 root）

```
adduser <username> <password>
```

### 3.5 修改密码

```
passwd <oldpassword> <newpassword>
```

示例：
```
passwd oldpass123 newpass456
```

> 修改密码后需使用新密码登录。

---

## 4. 文件和目录操作

### 4.1 列出当前目录内容

```
dir
```

### 4.2 创建文件

```
create <filename>
```

### 4.3 创建目录

```
mkdir <dirname>
```

### 4.4 删除文件或目录（支持递归删除）

```
delete <filename>
```

> 删除目录时，会递归删除其所有内容。

### 4.5 移动（重命名）文件或目录

```
mv <oldname> <newname>
```

> 功能与 `rename` 命令相同。

示例：
```
mv notes.txt archive.txt
```

### 4.6 复制文件

```
copy <source> <destination>
```

### 4.7 重命名文件

```
rename <oldname> <newname>
```

### 4.8 切换目录

```
cd <dirname>
```

支持特殊目录：
- `..` 返回上级
- `/` 根目录

### 4.9 查看文件信息

```
stat <filename>
```

### 4.10 文件搜索

```
find <pattern>
```

支持简单通配符 `*`。

---

## 5. 文件内容操作

### 5.1 打开文件

```
open <filename>
```

### 5.2 关闭文件

```
close <fd>
```

### 5.3 读取文件内容（多块文件支持）

```
read <fd> <size>
```

> 系统自动处理跨块读取。

### 5.4 写入文件内容（多块文件支持）

```
write <fd> <content>
```

> 单个文件最大支持 8 块（4096 字节）。

### 5.5 追加写入文件内容（多块文件支持）

```
append <filename> <content>
```

---

## 6. 权限管理

### 6.1 修改文件权限

```
chmod <filename> <permissions>
```

权限格式：
- `r`：读
- `w`：写
- `x`：执行
- `-`：无权限

示例：
```
chmod myfile.txt rw-
```

---

## 7. 文件系统操作

### 7.1 文件系统一致性检查

```
fsck
```

### 7.2 磁盘使用情况统计

```
du
```

---

## 8. 新增功能一览

✅ **多块数据存储**  
- 文件可自动扩展至最多 8 个块（4096 字节）。  
- 写入和读取时自动分配块。  

✅ **递归删除目录**  
- `delete` 命令可删除非空目录及全部子文件。

✅ **移动文件或目录**  
- `mv` 命令支持重命名或移动文件和目录。

✅ **修改用户密码**  
- `passwd` 命令可修改当前登录用户密码。

---

## 9. 示例会话

```
fs> login root root
登录成功，欢迎 root!

fs> mkdir projects
目录创建成功！

fs> cd projects
已切换到目录：projects

fs> create report.txt
文件创建成功！

fs> open report.txt
文件打开成功，文件描述符为：0

fs> write 0 "This is a test file spanning multiple blocks."
成功写入 45 字节。

fs> close 0
文件已关闭。

fs> dir
当前目录: projects
名称              类型    所有者    权限    大小    创建时间
report.txt        f       root      rwx     45      2025-07-03 14:10

fs> mv report.txt final_report.txt
文件已重命名(移动)为：final_report.txt

fs> passwd root newpassword
密码修改成功！

fs> delete projects
正在递归删除目录及其内容...
目录删除成功！

fs> exit
文件系统已安全退出。
```

---

## 10. 故障排除

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

---

## 11. 文件系统限制

- 最大文件名长度：28 字符
- 最大用户名长度：20 字符
- 最大密码长度：20 字符
- 最大 inode 数量：128
- 最大数据块数量：1024
- 最大用户数量：16
- 最大同时打开文件数：16
- 单个文件最大大小：4096 字节（8 块）

---

## 12. 开发信息

本系统使用 C 语言开发，模拟了 EXT2 文件系统的部分功能，包括：
- 超级块
- inode 表
- 数据块管理
- 位图
- 多用户系统
- 权限控制
- 命令行交互

---

如需进一步定制或扩展功能，请联系项目维护者。
