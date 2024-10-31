#include <linux/fs.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "simplefs.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s disk\n", argv[0]);
        return EXIT_FAILURE;
    }

    // open disk image
    int fd = open(argv[1], O_RDWR);
    if (fd == -1) {
        perror("open():");
        return EXIT_FAILURE;
    }

    // Get image size
    struct stat stat_buf;
    int ret = fstat(fd, &stat_buf);
    if (ret) {
        perror("fstat():");
        ret = EXIT_FAILURE;
        
        goto fclose;
    }

    // get block device size
    if ((stat_buf.st_mode & S_IFMT) == S_IFBLK) {
        long int blk_size = 0;
        ret = ioctl(fd, BLKGETSIZE64, &blk_size);
        
        if (ret != 0) {
            perror("BLKGETSIZE64:");
            ret = EXIT_FAILURE;
            
            goto fclose;
        }
        
        stat_buf.st_size = blk_size;
    }

    // verify if the file system image has sufficient size
    long int min_size = 100 * BLOCK_SIZE;
    if (stat_buf.st_size < min_size) {
        fprintf(stderr, "File is not large enough (size=%ld, min size=%ld)\n", stat_buf.st_size, min_size);
        ret = EXIT_FAILURE;
        goto fclose;
    }

    // write superblock from block 0
    struct simplefs_sb *sb = write_superblock(fd, &stat_buf);
    if (!sb) {
        perror("write_superblock():");
        ret = EXIT_FAILURE;
        
        goto fclose;
    }

    // write inode store blocks from block 1
    ret = write_inode_store(fd, sb);
    if (ret) {
        perror("write_inode_store():");
        ret = EXIT_FAILURE;

        goto free_sb;
    }

    // write inode free bitmap blocks
    ret = write_inode_free_block(fd, sb);
    if (ret) {
        perror("write_ifree_block()");
        ret = EXIT_FAILURE;
        
        goto free_sb;
    }

    // write block free bitmap blocks
    ret = write_block_free_block(fd, sb);
    if (ret) {
        perror("write_bfree_block()");
        ret = EXIT_FAILURE;
        
        goto free_sb;
    }

    // clear root index block
    ret = write_data_block(fd, sb);
    if (ret) {
        perror("write_data_block():");
        ret = EXIT_FAILURE;
        
        goto free_sb;
    }

free_sb:
    free(sb);
fclose:
    close(fd);

    return ret;
}
