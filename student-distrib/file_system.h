#include "types.h"
#pragma once

typedef struct dentry_t {
    uint8_t file_name[32];  // up to 32 chars, zero-padded
    uint32_t file_type;
    uint32_t inode_num;
    uint8_t reserved[24];  // reserved area
} dentry_t;

typedef struct boot_block_t {
    uint32_t num_dentries;
    uint32_t num_inodes;
    uint32_t num_data_blocks;
    uint8_t reserved[52];  // 52 reserved bytes
    dentry_t dentries[63];   // 63 possible dentries to have 
} boot_block_t;

typedef struct inode_t {
    uint32_t length;
    uint32_t data_blocks[1023];  // fill inode block with data block nums of 4 B == 1023 plus 4B for length
} inode_t;


void init_file_system(uint32_t file_system_start);  // get the pointers correctly (know what is starting address of fs_img, )
int32_t read_dentry_by_name(const uint8_t*, dentry_t* dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
int32_t get_directory_info(int32_t* type);

int32_t directory_open(const uint8_t* filename);
int32_t directory_close(int32_t fd);
int32_t directory_read(int32_t fd, void* buf, int32_t nbytes);
int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes);

int32_t file_open(const uint8_t* filename);
int32_t file_close(int32_t fd);
int32_t file_read(int32_t fd, void* buf, int32_t nbytes);
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);
