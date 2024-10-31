#ifndef _BITMAP
#define _BITMAP

#include <linux/bitmap.h>
#include "simplefs.h"

static inline uint32_t get_first_free_bit(unsigned long *freemap, unsigned long size, uint32_t len) {
    uint32_t bit;
    uint32_t prev = 0;
    uint32_t count = 0;
    
    for_each_set_bit (bit, freemap, size) {
        if (prev != bit - 1)
            count = 0;
        
        prev = bit;
        if (++count == len) {
            bitmap_clear(freemap, bit - len + 1, len);
            return bit - len + 1;
        }
    }
    
    return 0;
}

static inline uint32_t get_free_inode(struct simplefs_sb_info *sb_info) {
    uint32_t ret = get_first_free_bit(sb_info->inode_free_bitmap, sb_info->num_inode, 1);
    if (ret)
        sb_info->num_free_inode--;
        
    return ret;
}

static inline uint32_t get_free_block(struct super_block *sb, uint32_t len) {
    struct simplefs_sb_info *sb_info = sb->s_fs_info;
    uint32_t ret = get_first_free_bit(sb_info->block_free_bitmap, sb_info->num_block, len);
    
    if (!ret) // no enough free block
        return 0;

    sb_info->num_free_block -= len;
    for (uint32_t i = 0; i < len; i++) {
        struct buffer_head *bh = sb_bread(sb, ret + i);
        
        if (!bh) {
            pr_err("get_free_block: sb_bread failed for block %d\n", ret + i);
            sb_info->num_free_block += len;
            
            return -EIO;
        }
        
        memset(bh->b_data, 0, BLOCK_SIZE);
        mark_buffer_dirty(bh);
        sync_dirty_buffer(bh); // write buffer to disk
        brelse(bh);
    }
    
    return ret;
}

// mark from i-th bit in freemap as free
static inline int put_free_bit(unsigned long *freemap, unsigned long size, uint32_t i, uint32_t len) {
    if (i + len - 1 > size) // if i is greater than freemap size
        return -1;

    bitmap_set(freemap, i, len);

    return 0;
}

// mark inode as unused
static inline void put_inode(struct simplefs_sb_info *sb_info, uint32_t inode_no) {
    if (put_free_bit(sb_info->inode_free_bitmap, sb_info->num_inode, inode_no, 1))
        return;

    sb_info->num_free_inode++;
}

// mark block as unused
static inline void put_block(struct simplefs_sb_info *sb_info, uint32_t block_no, uint32_t len) {
    if (put_free_bit(sb_info->block_free_bitmap, sb_info->num_block, block_no, len))
        return;

    sb_info->num_free_block += len;
}

#endif
