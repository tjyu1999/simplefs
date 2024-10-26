#ifndef _SIMPLEFS
#define _SIMPLEFS

#define MAGIC             0xDEADCELL
#define NUM_BLOCK         0
#define BLOCK_SIZE        1 << 12
#define MAX_BLOCK_PER_EXT 8
#define FILENAME_LEN      255

#define INODE_PER_BLOCK  BLOCK_SIZE / sizeof(struct simplefs_inode)
#define MAX_EXT          (BLOCK_SIZE - sizeof(uint32_t)) / sizeof(struct simplefs_ext)
#define MAX_SIZE_PER_EXT MAX_BLOCK_PER_EXT * BLOCK_SIZE
#define MAX_FILESIZE     (uint64_t)MAX_BLOCK_PER_EXT * BLOCK_SIZE * MAX_EXT
#define FILE_PER_BLOCK   BLOCK_SIZE / sizeof(struct simplefs_file)
#define FILE_PER_EXT     FILE_PER_BLOCK * MAX_BLOCK_PER_EXT
#define MAX_SUBFILES     FILES_PER_EXT * MAX_EXT

#ifdef _KERNEL
#include <linux/jbd2.h>
#endif

struct simplefs_inode {
    uint32_t mode;
    uint32_t uid;
    uint32_t gid;
    uint32_t size;
    uint32_t link_cnt;
    uint32_t ext_block;
    char data[32];
};

struct simplefs_sb_info {
    uint32_t magic; // magic number
    uint32_t num_block;
    uint32_t num_inode;
    uint32_t num_inode_store_block;
    uint32_t num_inode_free_block;
    uint32_t num_block_free_block;
    
#ifdef _KERNEL
    journal_t *journal;
    struct bdev *journal_bdev;
    struct bdev_handle *journal_bdev_handle;
    unsigned long *inode_free_bitmap;
    unsigned long *block_free_bitmap;
#endif
};

#ifdef _KERNEL
#include <linux/version.h>
#define AT_LEAST(major, minor, rev)   LINUX_VERSION_CODE >= KERNEL_VERSION(major, minor, rev)
#define LESS_EQUAL(major, minor, rev) LINUX_VERSION_CODE <= KERNEL_VERSION(major, minor, rev)

struct simplefs_inode_info {
    uint32_t ext_block;
    char data[32];
    struct inode vfs_inode;
};

struct simplefs_ext_block {
    uint32_t logi_block;
    uint32_t phys_block;
    uint32_t ext_len;
};

struct simplefs_file {
    uint32_t inode;
    char filename[FILENAME_LEN];
};

struct simplefs_file_ext_block {
    uint32_t num_file;
    struct simplefs_ext_block extents[MAX_EXT];
};

struct simplefs_dir {
    struct simplefs_file files[FILE_PER_BLOCK];
}

// inode function
int simplefs_init_inode_cache(void);
void simplefs_destroy_inode_cache(void);
struct inode *simplefs_get_inode(struct super_block *sb, unsigned long index);

// superblock function
int simplefs_fill_sb(struct super_block *sb, void *data, int silent);
void simplefs_kill_sb(struct super_block *sb);

// dentry function
struct dentry *simplefs_mount(struct file_system_type *fs_type, int flags, const char *dev_name, void *data);

// file function
extern const struct file_operations simplefs_file_ops;
extern const struct file_operations simplefs_dir_ops;
extern const struct address_space_operations simplefs_aops;

// extent function
extern uint32_t simplefs_ext_search(struct simplefs_file_ext_block *index, uint32_t inode_block);

#endif // _KERNEL
#endif // _SIMPLEFS
