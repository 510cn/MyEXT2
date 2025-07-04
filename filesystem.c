#include "filesystem.h"

FileSystem fs;

// 函数声明
bool match_pattern(const char* filename, const char* pattern);
void get_file_path(int inode_id, char* path, int max_len);
void search_directory(int dir_inode, char* pattern, int* found);

// 初始化文件系统
void init_filesystem() {
    FILE* disk = fopen(DISK_NAME, "rb");
    if (disk == NULL) {
        printf("磁盘文件不存在，正在创建新的文件系统...\n");
        format_disk();
    } else {
        fclose(disk);
        load_filesystem();
    }
}

// 格式化磁盘
void format_disk() {
    fs.disk = fopen(DISK_NAME, "wb+");
    if (fs.disk == NULL) {
        printf("无法创建磁盘文件！\n");
        exit(1);
    }

    // 初始化超级块
    fs.super_block.total_blocks = MAX_BLOCKS;
    fs.super_block.free_blocks = MAX_BLOCKS - 1; // 第一个块用于超级块
    fs.super_block.total_inodes = MAX_INODES;
    fs.super_block.free_inodes = MAX_INODES - 1; // 第一个inode用于根目录
    fs.super_block.inode_bitmap_offset = sizeof(SuperBlock);
    fs.super_block.block_bitmap_offset = fs.super_block.inode_bitmap_offset + MAX_INODES/8;
    fs.super_block.inode_table_offset = fs.super_block.block_bitmap_offset + MAX_BLOCKS/8;
    fs.super_block.data_blocks_offset = fs.super_block.inode_table_offset + MAX_INODES * sizeof(Inode);

    // 初始化位图
    memset(fs.inode_bitmap, 0, MAX_INODES/8);
    memset(fs.block_bitmap, 0, MAX_BLOCKS/8);

    // 设置第一个块和第一个inode为已使用（根目录）
    fs.inode_bitmap[0] = 1;
    fs.block_bitmap[0] = 1;

    // 初始化inode表
    memset(fs.inode_table, 0, MAX_INODES * sizeof(Inode));
    
    // 创建根目录
    strcpy(fs.inode_table[0].filename, "/");
    fs.inode_table[0].owner_id = 0; // root用户
    strcpy(fs.inode_table[0].permission, "rwx");
    fs.inode_table[0].size = 0;
    fs.inode_table[0].data_blocks[0] = 0;
    fs.inode_table[0].block_count = 1;

    fs.inode_table[0].created = time(NULL);
    fs.inode_table[0].modified = time(NULL);
    fs.inode_table[0].accessed = time(NULL);
    fs.inode_table[0].is_directory = true;
    fs.inode_table[0].parent_inode = 0; // 根目录的父目录是自己

    // 初始化用户
    fs.user_count = 1;
    strcpy(fs.users[0].username, "root");
    strcpy(fs.users[0].password, "root");
    fs.users[0].uid = 0;

    // 初始化当前状态
    fs.current_user = -1; // 未登录
    fs.current_dir = 0;   // 根目录

    // 初始化打开文件表
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        fs.open_files[i].is_open = false;
    }

    // 保存到磁盘
    save_filesystem();
    printf("文件系统格式化完成！\n");
}

// 加载文件系统
void load_filesystem() {
    fs.disk = fopen(DISK_NAME, "rb+");
    if (fs.disk == NULL) {
        printf("无法打开磁盘文件！\n");
        exit(1);
    }

    // 读取超级块
    fread(&fs.super_block, sizeof(SuperBlock), 1, fs.disk);

    // 读取位图
    fseek(fs.disk, fs.super_block.inode_bitmap_offset, SEEK_SET);
    fread(fs.inode_bitmap, MAX_INODES/8, 1, fs.disk);

    fseek(fs.disk, fs.super_block.block_bitmap_offset, SEEK_SET);
    fread(fs.block_bitmap, MAX_BLOCKS/8, 1, fs.disk);

    // 读取inode表
    fseek(fs.disk, fs.super_block.inode_table_offset, SEEK_SET);
    fread(fs.inode_table, sizeof(Inode), MAX_INODES, fs.disk);

    // 读取用户信息（存储在数据区的第一个块）
    char buffer[BLOCK_SIZE];
    fseek(fs.disk, fs.super_block.data_blocks_offset, SEEK_SET);
    fread(buffer, BLOCK_SIZE, 1, fs.disk);
    
    fs.user_count = buffer[0]; // 第一个字节存储用户数量
    memcpy(fs.users, buffer + 1, fs.user_count * sizeof(User));

    // 初始化当前状态
    fs.current_user = -1; // 未登录
    fs.current_dir = 0;   // 根目录

    // 初始化打开文件表
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        fs.open_files[i].is_open = false;
    }

    printf("文件系统加载完成！\n");
}

// 保存文件系统
void save_filesystem() {
    if (fs.disk == NULL) {
        printf("磁盘文件未打开！\n");
        return;
    }

    // 写入超级块
    fseek(fs.disk, 0, SEEK_SET);
    fwrite(&fs.super_block, sizeof(SuperBlock), 1, fs.disk);

    // 写入位图
    fseek(fs.disk, fs.super_block.inode_bitmap_offset, SEEK_SET);
    fwrite(fs.inode_bitmap, MAX_INODES/8, 1, fs.disk);

    fseek(fs.disk, fs.super_block.block_bitmap_offset, SEEK_SET);
    fwrite(fs.block_bitmap, MAX_BLOCKS/8, 1, fs.disk);

    // 写入inode表
    fseek(fs.disk, fs.super_block.inode_table_offset, SEEK_SET);
    fwrite(fs.inode_table, sizeof(Inode), MAX_INODES, fs.disk);

    // 写入用户信息（存储在数据区的第一个块）
    char buffer[BLOCK_SIZE] = {0};
    buffer[0] = fs.user_count; // 第一个字节存储用户数量
    memcpy(buffer + 1, fs.users, fs.user_count * sizeof(User));
    
    fseek(fs.disk, fs.super_block.data_blocks_offset, SEEK_SET);
    fwrite(buffer, BLOCK_SIZE, 1, fs.disk);

    fflush(fs.disk);
}

// 分配一个空闲的inode
int allocate_inode() {
    for (int i = 0; i < MAX_INODES; i++) {
        if ((fs.inode_bitmap[i/8] & (1 << (i%8))) == 0) {
            fs.inode_bitmap[i/8] |= (1 << (i%8));
            fs.super_block.free_inodes--;
            return i;
        }
    }
    return -1; // 没有空闲inode
}

// 分配一个空闲的数据块
int allocate_block() {
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if ((fs.block_bitmap[i/8] & (1 << (i%8))) == 0) {
            fs.block_bitmap[i/8] |= (1 << (i%8));
            fs.super_block.free_blocks--;
            return i;
        }
    }
    return -1; // 没有空闲块
}

// 释放一个inode
void free_inode(int inode_id) {
    fs.inode_bitmap[inode_id/8] &= ~(1 << (inode_id%8));
    fs.super_block.free_inodes++;
}

// 释放一个数据块
void free_block(int block_id) {
    fs.block_bitmap[block_id/8] &= ~(1 << (block_id%8));
    fs.super_block.free_blocks++;
}

// 在当前目录中查找文件
int find_file(char* filename) {
    for (int i = 0; i < MAX_INODES; i++) {
        if ((fs.inode_bitmap[i/8] & (1 << (i%8))) && 
            fs.inode_table[i].parent_inode == fs.current_dir &&
            strcmp(fs.inode_table[i].filename, filename) == 0) {
            return i;
        }
    }
    return -1; // 文件不存在
}

// 检查文件权限
bool check_permission(int inode_id, char required_perm) {
    if (fs.current_user == 0) return true; // root用户有所有权限
    
    if (fs.inode_table[inode_id].owner_id == fs.current_user) {
        // 文件所有者
        if (required_perm == 'r' && strchr(fs.inode_table[inode_id].permission, 'r'))
            return true;
        if (required_perm == 'w' && strchr(fs.inode_table[inode_id].permission, 'w'))
            return true;
        if (required_perm == 'x' && strchr(fs.inode_table[inode_id].permission, 'x'))
            return true;
    }
    
    return false;
}

// 用户登录
int login(char* username, char* password) {
    for (int i = 0; i < fs.user_count; i++) {
        if (strcmp(fs.users[i].username, username) == 0 && 
            strcmp(fs.users[i].password, password) == 0) {
            fs.current_user = fs.users[i].uid;
            printf("登录成功，欢迎 %s!\n", username);
            return 0;
        }
    }
    printf("用户名或密码错误！\n");
    return -1;
}

// 用户登出
void logout() {
    if (fs.current_user == -1) {
        printf("您尚未登录！\n");
        return;
    }
    
    // 关闭所有打开的文件
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (fs.open_files[i].is_open) {
            fs.open_files[i].is_open = false;
        }
    }
    
    fs.current_user = -1;
    printf("已登出系统。\n");
}

// 列出当前目录内容
void list_directory() {
    if (fs.current_user == -1) {
        printf("请先登录！\n");
        return;
    }

    printf("当前目录: %s\n", fs.inode_table[fs.current_dir].filename);
    printf("名称\t\t类型\t所有者\t权限\t大小\t创建时间\n");
    
    for (int i = 0; i < MAX_INODES; i++) {
        if ((fs.inode_bitmap[i/8] & (1 << (i%8))) && 
            fs.inode_table[i].parent_inode == fs.current_dir) {
            
            char type = fs.inode_table[i].is_directory ? 'd' : 'f';
            char owner[MAX_USERNAME];
            strcpy(owner, fs.users[fs.inode_table[i].owner_id].username);
            
            char time_str[20];
            strftime(time_str, 20, "%Y-%m-%d %H:%M", localtime(&fs.inode_table[i].created));
            
            printf("%-16s\t%c\t%-8s\t%s\t%d\t%s\n", 
                   fs.inode_table[i].filename, 
                   type,
                   owner,
                   fs.inode_table[i].permission,
                   fs.inode_table[i].size,
                   time_str);
        }
    }
}

// 创建文件
int create_file(char* filename, bool is_directory) {
    if (fs.current_user == -1) {
        printf("请先登录！\n");
        return -1;
    }
    
    if (strlen(filename) >= MAX_FILENAME) {
        printf("文件名过长！\n");
        return -1;
    }
    
    if (find_file(filename) != -1) {
        printf("文件已存在！\n");
        return -1;
    }
    
    int inode_id = allocate_inode();
    if (inode_id == -1) {
        printf("inode资源已用尽！\n");
        return -1;
    }
    
    int block_id = allocate_block();
    if (block_id == -1) {
        free_inode(inode_id);
        printf("磁盘空间已用尽！\n");
        return -1;
    }
    
    // 初始化inode
    strcpy(fs.inode_table[inode_id].filename, filename);
    fs.inode_table[inode_id].owner_id = fs.current_user;
    strcpy(fs.inode_table[inode_id].permission, "rwx");
    fs.inode_table[inode_id].size = 0;
    fs.inode_table[inode_id].data_blocks[0] = block_id;
    fs.inode_table[inode_id].block_count = 1;

    fs.inode_table[inode_id].created = time(NULL);
    fs.inode_table[inode_id].modified = time(NULL);
    fs.inode_table[inode_id].accessed = time(NULL);
    fs.inode_table[inode_id].is_directory = is_directory;
    fs.inode_table[inode_id].parent_inode = fs.current_dir;
    
    // 如果是目录，初始化目录内容
    if (is_directory) {
        // 目录内容可以在需要时初始化
    }
    
    save_filesystem();
    printf("%s创建成功！\n", is_directory ? "目录" : "文件");
    return inode_id;
}

void delete_directory_recursive(int inode_id) {
    for (int i = 0; i < MAX_INODES; i++) {
        if ((fs.inode_bitmap[i/8] & (1 << (i%8))) &&
            fs.inode_table[i].parent_inode == inode_id) {
            if (fs.inode_table[i].is_directory) {
                delete_directory_recursive(i);
            } else {
                free_block(fs.inode_table[i].data_blocks[0]);
                free_inode(i);
            }
        }
    }
    for (int j = 0; j < fs.inode_table[inode_id].block_count; j++) {
        free_block(fs.inode_table[inode_id].data_blocks[j]);
    }
    free_inode(inode_id);
}


// 删除文件
int delete_file(char* filename) {
    if (fs.current_user == -1) {
        printf("请先登录！\n");
        return -1;
    }
    
    int inode_id = find_file(filename);
    if (inode_id == -1) {
        printf("文件不存在！\n");
        return -1;
    }
    
    // 检查权限
    if (!check_permission(inode_id, 'w')) {
        printf("权限不足，无法删除文件！\n");
        return -1;
    }
    
    // 检查是否为目录且非空
    if (fs.inode_table[inode_id].is_directory) {
        printf("正在递归删除目录及其内容...\n");
        delete_directory_recursive(inode_id);
        save_filesystem();
        printf("目录删除成功！\n");
        return 0;
    }

    
    // 检查文件是否已打开
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (fs.open_files[i].is_open && fs.open_files[i].inode_id == inode_id) {
            printf("文件已打开，请先关闭！\n");
            return -1;
        }
    }
    
    // 释放资源
    for (int j = 0; j < fs.inode_table[inode_id].block_count; j++) {
        free_block(fs.inode_table[inode_id].data_blocks[j]);
    }

    free_inode(inode_id);
    
    save_filesystem();
    printf("文件删除成功！\n");
    return 0;
}

// 打开文件
int open_file(char* filename) {
    if (fs.current_user == -1) {
        printf("请先登录！\n");
        return -1;
    }
    
    int inode_id = find_file(filename);
    if (inode_id == -1) {
        printf("文件不存在！\n");
        return -1;
    }
    
    if (fs.inode_table[inode_id].is_directory) {
        printf("无法打开目录！\n");
        return -1;
    }
    
    // 检查读权限
    if (!check_permission(inode_id, 'r')) {
        printf("权限不足，无法打开文件！\n");
        return -1;
    }
    
    // 查找空闲文件描述符
    int fd = -1;
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (!fs.open_files[i].is_open) {
            fd = i;
            break;
        }
    }
    
    if (fd == -1) {
        printf("打开文件数已达上限！\n");
        return -1;
    }
    
    // 更新打开文件表
    fs.open_files[fd].inode_id = inode_id;
    fs.open_files[fd].position = 0;
    fs.open_files[fd].is_open = true;
    
    // 更新访问时间
    fs.inode_table[inode_id].accessed = time(NULL);
    
    save_filesystem();
    printf("文件打开成功，文件描述符为：%d\n", fd);
    return fd;
}

// 关闭文件
void close_file(int fd) {
    if (fs.current_user == -1) {
        printf("请先登录！\n");
        return;
    }
    
    if (fd < 0 || fd >= MAX_OPEN_FILES || !fs.open_files[fd].is_open) {
        printf("无效的文件描述符！\n");
        return;
    }
    
    fs.open_files[fd].is_open = false;
    printf("文件已关闭。\n");
    
    save_filesystem();
}

// 读取文件内容
int read_file(int fd, char* buffer, int size) {
    if (fs.current_user == -1) {
        printf("请先登录！\n");
        return -1;
    }

    if (fd < 0 || fd >= MAX_OPEN_FILES || !fs.open_files[fd].is_open) {
        printf("无效的文件描述符！\n");
        return -1;
    }

    int inode_id = fs.open_files[fd].inode_id;
    Inode* inode = &fs.inode_table[inode_id];

    if (!check_permission(inode_id, 'r')) {
        printf("权限不足，无法读取文件！\n");
        return -1;
    }

    int bytes_read = 0;
    int remaining = size;
    int pos = fs.open_files[fd].position;

    while (remaining > 0 && pos < inode->size) {
        int block_index = pos / BLOCK_SIZE;
        int offset_in_block = pos % BLOCK_SIZE;

        int to_read = BLOCK_SIZE - offset_in_block;
        if (to_read > remaining) to_read = remaining;
        if (to_read > inode->size - pos) to_read = inode->size - pos;

        fseek(fs.disk, fs.super_block.data_blocks_offset + inode->data_blocks[block_index]*BLOCK_SIZE + offset_in_block, SEEK_SET);
        fread(buffer + bytes_read, 1, to_read, fs.disk);

        bytes_read += to_read;
        pos += to_read;
        remaining -= to_read;
    }

    fs.open_files[fd].position = pos;
    inode->accessed = time(NULL);

    save_filesystem();
    return bytes_read;
}

// 写入文件内容
int write_file(int fd, char* buffer, int size) {
    if (fs.current_user == -1) {
        printf("请先登录！\n");
        return -1;
    }

    if (fd < 0 || fd >= MAX_OPEN_FILES || !fs.open_files[fd].is_open) {
        printf("无效的文件描述符！\n");
        return -1;
    }

    int inode_id = fs.open_files[fd].inode_id;

    // 检查写权限
    if (!check_permission(inode_id, 'w')) {
        printf("权限不足，无法写入文件！\n");
        return -1;
    }

    Inode* inode = &fs.inode_table[inode_id];

    int bytes_written = 0;
    int remaining = size;

    // 写入数据块
    while (remaining > 0) {
        int current_block = inode->block_count - 1;
        if (current_block >= 8) {
            printf("文件已达到最大块数，无法写入更多数据！\n");
            break;
        }

        int offset_in_block = inode->size % BLOCK_SIZE;
        int free_space = BLOCK_SIZE - offset_in_block;
        int to_write = (remaining < free_space) ? remaining : free_space;

        // 定位写入位置
        fseek(fs.disk, fs.super_block.data_blocks_offset + inode->data_blocks[current_block]*BLOCK_SIZE + offset_in_block, SEEK_SET);
        fwrite(buffer + bytes_written, 1, to_write, fs.disk);

        inode->size += to_write;
        bytes_written += to_write;
        remaining -= to_write;

        // 如果本块写满了并且还有数据，分配新块
        if (offset_in_block + to_write == BLOCK_SIZE && remaining > 0) {
            int new_block = allocate_block();
            if (new_block == -1) {
                printf("磁盘空间不足，写入中断！\n");
                break;
            }
            inode->data_blocks[inode->block_count++] = new_block;
        }
    }

    inode->modified = time(NULL);
    fs.open_files[fd].position = inode->size;

    fflush(fs.disk);
    save_filesystem();
    return bytes_written;
}

// 修改文件权限
int change_permission(char* filename, char* new_perm) {
    if (fs.current_user == -1) {
        printf("请先登录！\n");
        return -1;
    }
    
    int inode_id = find_file(filename);
    if (inode_id == -1) {
        printf("文件不存在！\n");
        return -1;
    }
    
    // 只有文件所有者或root用户可以修改权限
    if (fs.current_user != 0 && fs.current_user != fs.inode_table[inode_id].owner_id) {
        printf("权限不足，只有文件所有者或root用户可以修改权限！\n");
        return -1;
    }
    
    // 检查权限格式是否正确
    if (strlen(new_perm) != 3 || 
        (new_perm[0] != 'r' && new_perm[0] != '-') ||
        (new_perm[1] != 'w' && new_perm[1] != '-') ||
        (new_perm[2] != 'x' && new_perm[2] != '-')) {
        printf("权限格式错误，应为rwx或r-x等形式！\n");
        return -1;
    }
    
    // 构建新的权限字符串
    char perm[4] = {0};
    if (new_perm[0] == 'r') perm[0] = 'r';
    if (new_perm[1] == 'w') perm[1] = 'w';
    if (new_perm[2] == 'x') perm[2] = 'x';
    
    // 更新权限
    strcpy(fs.inode_table[inode_id].permission, perm);
    
    save_filesystem();
    printf("文件权限已修改为：%s\n", perm);
    return 0;
}

// 退出系统
void exit_system() {
    if (fs.disk != NULL) {
        save_filesystem();
        fclose(fs.disk);
        fs.disk = NULL;
    }
    printf("文件系统已安全退出。\n");
}

// 移动/重命名文件
int move_file(char* oldname, char* newname) {
    if (fs.current_user == -1) {
        printf("请先登录！\n");
        return -1;
    }

    if (strlen(newname) >= MAX_FILENAME) {
        printf("新名称过长！\n");
        return -1;
    }

    int inode_id = find_file(oldname);
    if (inode_id == -1) {
        printf("文件不存在！\n");
        return -1;
    }

    if (!check_permission(inode_id, 'w')) {
        printf("权限不足，无法移动文件！\n");
        return -1;
    }

    if (find_file(newname) != -1) {
        printf("目标名称已存在！\n");
        return -1;
    }

    strcpy(fs.inode_table[inode_id].filename, newname);
    fs.inode_table[inode_id].modified = time(NULL);

    save_filesystem();
    printf("文件已重命名(移动)为：%s\n", newname);
    return 0;
}

// 修改密码
int change_password(char* oldpass, char* newpass) {
    if (fs.current_user == -1) {
        printf("请先登录！\n");
        return -1;
    }

    if (strlen(newpass) >= MAX_PASSWORD) {
        printf("新密码过长！\n");
        return -1;
    }

    int uid = fs.current_user;
    if (strcmp(fs.users[uid].password, oldpass) != 0) {
        printf("旧密码错误！\n");
        return -1;
    }

    strcpy(fs.users[uid].password, newpass);
    save_filesystem();
    printf("密码修改成功！\n");
    return 0;
}


// 切换目录
int change_directory(char* dirname) {
    if (fs.current_user == -1) {
        printf("请先登录！\n");
        return -1;
    }
    
    // 处理特殊情况：返回上级目录
    if (strcmp(dirname, "..") == 0) {
        if (fs.current_dir != 0) { // 不是根目录
            fs.current_dir = fs.inode_table[fs.current_dir].parent_inode;
            printf("已切换到上级目录。\n");
            return 0;
        } else {
            printf("已经在根目录，无法返回上级目录。\n");
            return -1;
        }
    }
    
    // 处理特殊情况：当前目录
    if (strcmp(dirname, ".") == 0) {
        return 0; // 不做任何改变
    }
    
    // 处理特殊情况：根目录
    if (strcmp(dirname, "/") == 0) {
        fs.current_dir = 0;
        printf("已切换到根目录。\n");
        return 0;
    }
    
    // 查找目录
    int inode_id = find_file(dirname);
    if (inode_id == -1) {
        printf("目录不存在！\n");
        return -1;
    }
    
    // 检查是否为目录
    if (!fs.inode_table[inode_id].is_directory) {
        printf("%s 不是目录！\n", dirname);
        return -1;
    }
    
    // 检查执行权限
    if (!check_permission(inode_id, 'x')) {
        printf("权限不足，无法进入目录！\n");
        return -1;
    }
    
    fs.current_dir = inode_id;
    printf("已切换到目录：%s\n", dirname);
    return 0;
}

// 复制文件
int copy_file(char* source, char* destination) {
    if (fs.current_user == -1) {
        printf("请先登录！\n");
        return -1;
    }
    
    // 查找源文件
    int src_inode_id = find_file(source);
    if (src_inode_id == -1) {
        printf("源文件不存在！\n");
        return -1;
    }
    
    // 检查源文件是否为目录
    if (fs.inode_table[src_inode_id].is_directory) {
        printf("不支持复制目录！\n");
        return -1;
    }
    
    // 检查读权限
    if (!check_permission(src_inode_id, 'r')) {
        printf("权限不足，无法读取源文件！\n");
        return -1;
    }
    
    // 检查目标文件是否已存在
    int dst_inode_id = find_file(destination);
    if (dst_inode_id != -1) {
        printf("目标文件已存在！\n");
        return -1;
    }
    
    // 创建目标文件
    dst_inode_id = create_file(destination, false);
    if (dst_inode_id == -1) {
        return -1;
    }
    
    // 打开源文件
    int src_fd = open_file(source);
    if (src_fd == -1) {
        delete_file(destination);
        return -1;
    }
    
    // 打开目标文件
    int dst_fd = open_file(destination);
    if (dst_fd == -1) {
        close_file(src_fd);
        delete_file(destination);
        return -1;
    }
    
    // 复制内容
    char buffer[BLOCK_SIZE];
    int bytes_read;
    int total_copied = 0;
    
    // 读取源文件内容并写入目标文件
    bytes_read = read_file(src_fd, buffer, BLOCK_SIZE);
    if (bytes_read > 0) {
        int bytes_written = write_file(dst_fd, buffer, bytes_read);
        if (bytes_written != bytes_read) {
            printf("写入目标文件时出错！\n");
            close_file(src_fd);
            close_file(dst_fd);
            delete_file(destination);
            return -1;
        }
        total_copied += bytes_written;
    }
    
    // 关闭文件
    close_file(src_fd);
    close_file(dst_fd);
    
    printf("文件复制成功，共复制 %d 字节。\n", total_copied);
    return 0;
}

// 创建新用户
int create_user(char* username, char* password) {
    // 只有root用户可以创建新用户
    if (fs.current_user != 0) {
        printf("只有root用户可以创建新用户！\n");
        return -1;
    }
    
    if (strlen(username) >= MAX_USERNAME || strlen(password) >= MAX_PASSWORD) {
        printf("用户名或密码过长！\n");
        return -1;
    }
    
    // 检查用户名是否已存在
    for (int i = 0; i < fs.user_count; i++) {
        if (strcmp(fs.users[i].username, username) == 0) {
            printf("用户名已存在！\n");
            return -1;
        }
    }
    
    // 检查用户数量是否已达到上限
    if (fs.user_count >= MAX_USERS) {
        printf("用户数量已达到上限！\n");
        return -1;
    }
    
    // 创建新用户
    fs.users[fs.user_count].uid = fs.user_count;
    strcpy(fs.users[fs.user_count].username, username);
    strcpy(fs.users[fs.user_count].password, password);
    
    fs.user_count++;
    
    save_filesystem();
    printf("用户 %s 创建成功！\n", username);
    return 0;
}

// 显示文件信息
void show_file_info(char* filename) {
    if (fs.current_user == -1) {
        printf("请先登录！\n");
        return;
    }
    
    int inode_id = find_file(filename);
    if (inode_id == -1) {
        printf("文件不存在！\n");
        return;
    }
    
    char type = fs.inode_table[inode_id].is_directory ? 'd' : 'f';
    char owner[MAX_USERNAME];
    strcpy(owner, fs.users[fs.inode_table[inode_id].owner_id].username);
    
    char created_time[20];
    char modified_time[20];
    char accessed_time[20];
    
    strftime(created_time, 20, "%Y-%m-%d %H:%M", localtime(&fs.inode_table[inode_id].created));
    strftime(modified_time, 20, "%Y-%m-%d %H:%M", localtime(&fs.inode_table[inode_id].modified));
    strftime(accessed_time, 20, "%Y-%m-%d %H:%M", localtime(&fs.inode_table[inode_id].accessed));
    
    printf("\n文件信息：\n");
    printf("名称: %s\n", fs.inode_table[inode_id].filename);
    printf("类型: %s\n", fs.inode_table[inode_id].is_directory ? "目录" : "文件");
    printf("所有者: %s\n", owner);
    printf("权限: %s\n", fs.inode_table[inode_id].permission);
    printf("大小: %d 字节\n", fs.inode_table[inode_id].size);
    printf("创建时间: %s\n", created_time);
    printf("修改时间: %s\n", modified_time);
    printf("访问时间: %s\n", accessed_time);
    printf("Inode ID: %d\n", inode_id);
    printf("数据块ID: ");
    for (int j = 0; j < fs.inode_table[inode_id].block_count; j++) {
        printf("%d ", fs.inode_table[inode_id].data_blocks[j]);
    }
    printf("\n");

}

// 追加写入文件内容
int append_file(char* filename, char* content) {
    if (fs.current_user == -1) {
        printf("请先登录！\n");
        return -1;
    }
    
    int inode_id = find_file(filename);
    if (inode_id == -1) {
        printf("文件不存在！\n");
        return -1;
    }
    
    if (fs.inode_table[inode_id].is_directory) {
        printf("无法写入目录！\n");
        return -1;
    }
    
    // 检查写权限
    if (!check_permission(inode_id, 'w')) {
        printf("权限不足，无法写入文件！\n");
        return -1;
    }
    
    // 打开文件
    int fd = -1;
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (!fs.open_files[i].is_open) {
            fd = i;
            break;
        }
    }
    
    if (fd == -1) {
        printf("打开文件数已达上限！\n");
        return -1;
    }
    
    // 设置为追加模式（移动到文件末尾）
    fs.open_files[fd].inode_id = inode_id;
    fs.open_files[fd].position = fs.inode_table[inode_id].size;
    fs.open_files[fd].is_open = true;
    
    // 检查是否超出块大小限制
    int content_len = strlen(content);
    if (fs.open_files[fd].position + content_len > BLOCK_SIZE) {
        content_len = BLOCK_SIZE - fs.open_files[fd].position;
        printf("追加数据已截断，最大可写入 %d 字节。\n", content_len);
        if (content_len <= 0) {
            printf("文件已满，无法写入更多数据！\n");
            fs.open_files[fd].is_open = false;
            return 0;
        }
    }
    
    // 写入数据
    int block_id = fs.inode_table[inode_id].data_blocks[fs.inode_table[inode_id].block_count - 1];

    fseek(fs.disk, fs.super_block.data_blocks_offset + block_id * BLOCK_SIZE + fs.open_files[fd].position, SEEK_SET);
    int bytes_written = fwrite(content, 1, content_len, fs.disk);
    
    // 更新文件大小
    fs.inode_table[inode_id].size += bytes_written;
    
    // 更新修改时间
    fs.inode_table[inode_id].modified = time(NULL);
    
    // 关闭文件
    fs.open_files[fd].is_open = false;
    
    save_filesystem();
    printf("成功追加 %d 字节到文件。\n", bytes_written);
    return bytes_written;
}

// 文件系统一致性检查
int check_filesystem() {
    printf("开始文件系统一致性检查...\n");
    int errors = 0;
    
    // 检查inode位图与实际inode使用情况是否一致
    for (int i = 0; i < MAX_INODES; i++) {
        bool is_used = (fs.inode_bitmap[i/8] & (1 << (i%8))) != 0;
        
        // 检查根目录inode必须存在
        if (i == 0 && !is_used) {
            printf("错误：根目录inode未标记为使用！\n");
            fs.inode_bitmap[0] |= 1; // 修复：标记为使用
            errors++;
        }
        
        // 检查已使用的inode是否有有效的文件名
        if (is_used && i > 0) {
            if (strlen(fs.inode_table[i].filename) == 0) {
                printf("错误：inode %d 被标记为使用但没有文件名！\n", i);
                fs.inode_bitmap[i/8] &= ~(1 << (i%8)); // 修复：标记为未使用
                errors++;
            }
        }
    }
    
    // 检查数据块位图与实际数据块使用情况是否一致
    for (int i = 0; i < MAX_INODES; i++) {
        if ((fs.inode_bitmap[i/8] & (1 << (i%8)))) {
            int block_id = fs.inode_table[i].data_blocks[0]; // 这里只检查第一个块

            
            // 检查数据块是否在有效范围内
            if (block_id < 0 || block_id >= MAX_BLOCKS) {
                printf("错误：inode %d 引用了无效的数据块 %d！\n", i, block_id);
                // 分配一个新的数据块
                int new_block = allocate_block();
                if (new_block != -1) {
                    fs.inode_table[i].data_blocks[0] = new_block;
                    fs.inode_table[i].block_count = 1;

                    printf("修复：为inode %d 分配了新的数据块 %d\n", i, new_block);
                }
                errors++;
            } else {
                // 检查数据块是否被标记为使用
                if (!(fs.block_bitmap[block_id/8] & (1 << (block_id%8)))) {
                    printf("错误：inode %d 使用的数据块 %d 未标记为使用！\n", i, block_id);
                    fs.block_bitmap[block_id/8] |= (1 << (block_id%8)); // 修复：标记为使用
                    errors++;
                }
            }
        }
    }
    
    // 检查目录结构的一致性
    for (int i = 0; i < MAX_INODES; i++) {
        if ((fs.inode_bitmap[i/8] & (1 << (i%8))) && fs.inode_table[i].is_directory) {
            int parent_id = fs.inode_table[i].parent_inode;
            
            // 检查父目录是否存在且是目录
            if (parent_id != i) { // 根目录的父目录是自己
                if (!(fs.inode_bitmap[parent_id/8] & (1 << (parent_id%8)))) {
                    printf("错误：目录 %s (inode %d) 的父目录 (inode %d) 不存在！\n", 
                           fs.inode_table[i].filename, i, parent_id);
                    // 修复：将父目录设置为根目录
                    fs.inode_table[i].parent_inode = 0;
                    errors++;
                } else if (!fs.inode_table[parent_id].is_directory) {
                    printf("错误：目录 %s (inode %d) 的父inode %d 不是目录！\n", 
                           fs.inode_table[i].filename, i, parent_id);
                    // 修复：将父目录设置为根目录
                    fs.inode_table[i].parent_inode = 0;
                    errors++;
                }
            }
        }
    }
    
    // 更新超级块中的空闲块和inode计数
    int free_inodes = 0;
    int free_blocks = 0;
    
    for (int i = 0; i < MAX_INODES; i++) {
        if (!(fs.inode_bitmap[i/8] & (1 << (i%8)))) {
            free_inodes++;
        }
    }
    
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (!(fs.block_bitmap[i/8] & (1 << (i%8)))) {
            free_blocks++;
        }
    }
    
    fs.super_block.free_inodes = free_inodes;
    fs.super_block.free_blocks = free_blocks;
    
    if (errors > 0) {
        printf("文件系统检查完成，发现并修复了 %d 个错误。\n", errors);
        save_filesystem();
    } else {
        printf("文件系统检查完成，未发现错误。\n");
    }
    
    return errors;
}

// 文件搜索（支持简单的通配符 * 匹配）
void search_file(char* pattern) {
    if (fs.current_user == -1) {
        printf("请先登录！\n");
        return;
    }
    
    printf("搜索结果：\n");
    printf("名称\t\t类型\t所有者\t路径\n");
    
    int found = 0;
    
    // 从根目录开始搜索
    search_directory(0, pattern, &found);
    
    if (found == 0) {
        printf("未找到匹配的文件或目录。\n");
    } else {
        printf("共找到 %d 个匹配项。\n", found);
    }
}

// 文件重命名
int rename_file(char* oldname, char* newname) {
    if (fs.current_user == -1) {
        printf("请先登录！\n");
        return -1;
    }
    
    if (strlen(newname) >= MAX_FILENAME) {
        printf("新文件名过长！\n");
        return -1;
    }
    
    int inode_id = find_file(oldname);
    if (inode_id == -1) {
        printf("文件不存在！\n");
        return -1;
    }
    
    // 检查写权限
    if (!check_permission(inode_id, 'w')) {
        printf("权限不足，无法重命名文件！\n");
        return -1;
    }
    
    // 检查新名称是否已存在
    if (find_file(newname) != -1) {
        printf("目标文件名已存在！\n");
        return -1;
    }
    
    // 检查文件是否已打开
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (fs.open_files[i].is_open && fs.open_files[i].inode_id == inode_id) {
            printf("文件已打开，请先关闭！\n");
            return -1;
        }
    }
    
    // 更新文件名
    strcpy(fs.inode_table[inode_id].filename, newname);
    
    // 更新修改时间
    fs.inode_table[inode_id].modified = time(NULL);
    
    save_filesystem();
    printf("文件重命名成功！\n");
    return 0;
}

// 磁盘使用情况统计
void disk_usage() {
    if (fs.current_user == -1) {
        printf("请先登录！\n");
        return;
    }
    
    int total_inodes = fs.super_block.total_inodes;
    int free_inodes = fs.super_block.free_inodes;
    int used_inodes = total_inodes - free_inodes;
    
    int total_blocks = fs.super_block.total_blocks;
    int free_blocks = fs.super_block.free_blocks;
    int used_blocks = total_blocks - free_blocks;
    
    int total_files = 0;
    int total_dirs = 0;
    int total_size = 0;
    
    // 统计文件和目录数量
    for (int i = 0; i < MAX_INODES; i++) {
        if (fs.inode_bitmap[i/8] & (1 << (i%8))) {
            if (fs.inode_table[i].is_directory) {
                total_dirs++;
            } else {
                total_files++;
                total_size += fs.inode_table[i].size;
            }
        }
    }
    
    printf("\n磁盘使用情况统计：\n");
    printf("总inode数量: %d\n", total_inodes);
    printf("已使用inode: %d (%.1f%%)\n", used_inodes, (float)used_inodes / total_inodes * 100);
    printf("空闲inode: %d (%.1f%%)\n", free_inodes, (float)free_inodes / total_inodes * 100);
    
    printf("\n总数据块数量: %d\n", total_blocks);
    printf("已使用数据块: %d (%.1f%%)\n", used_blocks, (float)used_blocks / total_blocks * 100);
    printf("空闲数据块: %d (%.1f%%)\n", free_blocks, (float)free_blocks / total_blocks * 100);
    
    printf("\n文件总数: %d\n", total_files);
    printf("目录总数: %d\n", total_dirs);
    printf("文件系统总大小: %d 字节\n", total_blocks * BLOCK_SIZE);
    printf("已使用空间: %d 字节 (%.1f%%)\n", total_size, (float)total_size / (total_blocks * BLOCK_SIZE) * 100);
    
    // 按用户统计文件数量和大小
    printf("\n按用户统计：\n");
    printf("用户名\t\t文件数\t目录数\t总大小(字节)\n");
    
    for (int u = 0; u < fs.user_count; u++) {
        int user_files = 0;
        int user_dirs = 0;
        int user_size = 0;
        
        for (int i = 0; i < MAX_INODES; i++) {
            if ((fs.inode_bitmap[i/8] & (1 << (i%8))) && 
                fs.inode_table[i].owner_id == u) {
                
                if (fs.inode_table[i].is_directory) {
                    user_dirs++;
                } else {
                    user_files++;
                    user_size += fs.inode_table[i].size;
                }
            }
        }
        
        printf("%-16s\t%d\t%d\t%d\n", 
               fs.users[u].username, 
               user_files, 
               user_dirs, 
               user_size);
    }
}

// 简单的通配符匹配函数
bool match_pattern(const char* filename, const char* pattern) {
    // 如果模式中没有通配符，直接比较字符串
    if (strchr(pattern, '*') == NULL) {
        return strcmp(filename, pattern) == 0;
    }

    const char* star = strchr(pattern, '*');
    size_t prefix_len = star - pattern;

    // 检查前缀是否匹配
    if (strncmp(filename, pattern, prefix_len) != 0) {
        return false;
    }

    // 如果 * 是最后一个字符，则任何前缀匹配的都符合
    if (*(star + 1) == '\0') {
        return true;
    }

    // 否则，检查后缀是否匹配
    size_t filename_len = strlen(filename);
    size_t suffix_len = strlen(pattern) - prefix_len - 1;

    if (filename_len < prefix_len + suffix_len) {
        return false;
    }

    return strcmp(filename + filename_len - suffix_len, star + 1) == 0;
}

// 获取文件的完整路径
void get_file_path(int inode_id, char* path, int max_len) {
    if (inode_id == 0) {
        strcpy(path, "/");
        return;
    }

    char temp_path[MAX_FILENAME * 10] = {0}; // 足够大的缓冲区
    int current = inode_id;

    // 从当前节点向上遍历到根
    while (current != 0) {
        char name_buf[MAX_FILENAME + 1] = {0};
        strcpy(name_buf, fs.inode_table[current].filename);

        // 将当前文件名添加到路径前面
        memmove(temp_path + strlen(name_buf) + 1, temp_path, strlen(temp_path) + 1);
        temp_path[0] = '/';
        memcpy(temp_path + 1, name_buf, strlen(name_buf));

        current = fs.inode_table[current].parent_inode;
    }

    // 如果路径为空，设置为根目录
    if (strlen(temp_path) == 0) {
        strcpy(temp_path, "/");
    }

    strncpy(path, temp_path, max_len - 1);
    path[max_len - 1] = '\0';
}

// 递归搜索目录
void search_directory(int dir_inode, char* pattern, int* found) {
    for (int i = 0; i < MAX_INODES; i++) {
        if ((fs.inode_bitmap[i/8] & (1 << (i%8))) &&
            fs.inode_table[i].parent_inode == dir_inode) {

            // 检查文件名是否匹配模式
            if (match_pattern(fs.inode_table[i].filename, pattern)) {
                char path[MAX_FILENAME * 10] = {0};
                get_file_path(i, path, sizeof(path));

                char type = fs.inode_table[i].is_directory ? 'd' : 'f';
                char owner[MAX_USERNAME];
                strcpy(owner, fs.users[fs.inode_table[i].owner_id].username);

                printf("%-16s\t%c\t%-8s\t%s\n",
                       fs.inode_table[i].filename,
                       type,
                       owner,
                       path);
                (*found)++;
            }

            // 如果是目录，递归搜索
            if (fs.inode_table[i].is_directory) {
                search_directory(i, pattern, found);
            }
        }
    }
}
