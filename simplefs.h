#ifndef _SIMPLEFS
#define _SIMPLEFS

#define MAGIC             0xDEADCELL
#define NUM_BLOCK         0
#define BLOCK_SIZE        1 << 12
#define MAX_BLOCK_PER_EXT 8
#define FILENAME_LEN      255

#define INODE_PER_BLOCK  BLOCK_SIZE / sizeof(struct sfs_inode)
#define MAX_EXT          (BLOCK_SIZE - sizeof(uint32_t)) / sizeof(struct sfs_ext_block)
#define MAX_SIZE_PER_EXT MAX_BLOCK_PER_EXT * BLOCK_SIZE
#define MAX_FILESIZE     (uint64_t)MAX_BLOCK_PER_EXT * BLOCK_SIZE * MAX_EXT
#define FILE_PER_BLOCK   BLOCK_SIZE / sizeof(struct sfs_file)
#define FILE_PER_EXT     FILE_PER_BLOCK * MAX_BLOCK_PER_EXT
#define MAX_SUBFILES     FILES_PER_EXT * MAX_EXT

#ifdef _KERNEL
#include <linux/jbd2.h>
#endif

struct sfs_inode {
    uint32_t ext_block;
    char data[32];
};

struct sfs_sb_info {
    uint32_t magic;
    uint32_t num_inode_store_block;
    uint32_t num_inode_free_bitmap_block;
    uint32_t num_block_free_bitmap_block;
    
    #ifdef _KERNEL
        journal_t *journal;
        struct bdev *journal_bdev;
        struct bdev_handle *journal_bdev_handle;
        unsigned long *inode_free_bitmap;
        unsigned long *bitmap_free_bitmap;
    #endif
};

#ifdef _KERNEL
#include <linux/version.h>
#define AT_LEAST(major, minor, rev)   LINUX_VERSION_CODE >= KERNEL_VERSION(major, minor, rev)
#define LESS_EQUAL(major, minor, rev) LINUX_VERSION_CODE <= KERNEL_VERSION(major, minor, rev)

struct sfs_vfs_inode {
    uint32_t ext_block;
    char data[32];
    struct inode vfs_inode;
};

struct sfs_ext_block {
    uint32_t logi_block;
    uint32_t phys_block;
    uint32_t ext_len;
};

struct sfs_file {
    uint32_t inode;
    char filename[FILENAME_LEN];
};

struct sfs_file_ext_block {
    uint32_t num_file;
    struct sfs_ext_block ext_block[MAX_EXT];
};

struct sfs_dir {
    struct sfs_file file[FILE_PER_BLOCK];
}

#endif
#endif
