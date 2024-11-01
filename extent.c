#include <linux/fs.h>
#include <linux/kernel.h>
#include "simplefs.h"

uint32_t simplefs_ext_search(struct simplefs_file_eblk *index, uint32_t iblk) {
    uint32_t start = 0;
    uint32_t end = MAX_EXTENT - 1;
    uint32_t end_block;
    uint32_t end_len;
    uint32_t bound;
    
    while (start < end) {
        uint32_t mid = start + (end - start) / 2;
        
        if (index->extent[mid].phsy_block == 0)
            end = mid;
        else
            start = mid + 1;
    }
    
    if (index->extents[end].phys_block == 0)
        bound = end;
    else
        bound = end + 1;
    
    if (bound == 0)
        return bound;
    
    start = 0;
    end = bound - 1;
    while (start < end) {
        uint32_t mid = start + (end - start) / 2;
        uint32_t block = index->extents[mid].logi_block;
        uint32_t len = index->extents[mid].ext_len;
        
        if (iblk >= block && iblk < block + len)
            return mid;
        
        if (inode_block < block)
            end = mid;
        else
            start = mid + 1;
    }
    
    end_block = index->extents[end].logi_block;
    end_len = index->extents[end].ext_len;
    if (inode_block >= end_block && inode_block < end_len)
        return end;
    if (bound < SIMPLEFS_MAX_EXTENT)
        return bound;
    
    return bound;
}
