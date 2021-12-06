#include "file_system.h"
#include "lib.h"
#include "system_call.h"
#define BLOCK_SIZE 4096
#define DENTRY_SIZE 64
#define FILE_NAME_SIZE 32
#define INODE_ENTRY_SIZE 4

// file_descriptor_t fd_array[8];
uint32_t* FS_start;
boot_block_t* boot_block;

/***********************************/
dentry_t fs_dentry;
dentry_t* fs_dentry_ptr = &fs_dentry;

dentry_t d_dentry;
dentry_t* d_dentry_ptr = &d_dentry;

uint32_t offset;
uint32_t index;
/************************************/


/* 
 * init_file_system
 * Inputs: int32_t file_system_start
 * Return Value: none
 * Function: initializes file systems by setting up...  
 */
void init_file_system(uint32_t file_system_start) {
    boot_block = (boot_block_t*)file_system_start;
}

/* 
 * read_dentry_by_name
 * Inputs: const uint8_t* fname, dentry_t* dentry
 * Return Value: 
 * Function: Returns -1 on failure, indicating a non-existent file.
 *           If successful, fills dentry_t block with file name, file type,
 *           and inode number for the file before returning 0.
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry) {
    int i;
    uint32_t len;
    // uint32_t len2;
    // boot_block = FS_start; // set the boot block address to the start of file_sys img
    dentry_t* curr_dentry = (dentry_t*)boot_block->dentries;  // point to the start of the dentries in boot block
    // uint32_t num_d = boot_block->num_dentries;  // get the number of dentries
    for (i = 0; i < 63; i++) {   // 63 is the max number of dentries
        len = strlen((const int8_t*)fname);
        // don't let user submit file with name longer than 32 chars
        if (len > 32) {
            return -1;
        }

        int compare = strncmp((const int8_t*)curr_dentry->file_name, (const int8_t*)fname, 32);
        if (compare == 0) {     // check to see if dentry's file name matches arg
            memcpy(dentry, curr_dentry, sizeof(dentry_t));  // if so, copy that dentry
            return 0;
        }
        curr_dentry += 1;  // move to the next dentry
    }
    return -1;
}

/* 
 * read_dentry_by_index
 * Inputs: uint32_t index, dentry_t* dentry
 * Return Value: 
 * Function: Returns -1 on failure, indicating a non-existent index.
 *           If successful, fills dentry_t block with file name, file type,
 *           and inode number for the file before returning 0.
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry) {
    // boot_block = FS_start;
    dentry_t* dentries_start = (dentry_t*)boot_block->dentries;
    uint32_t dentry_count = boot_block->num_dentries;  // get the number of inodes
    if (index > dentry_count) {
        return -1;  // if invalid index, return -1
    }
    dentry_t* curr_dentry = dentries_start + index; //DENTRY_SIZE * index;  // move to the correct dentry based on index
    memcpy(dentry, curr_dentry, sizeof(dentry_t));  // copy that dentry
    return 0;
}

/* 
 * read_data
 * Inputs: uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length
 * Return Value: 
 * Function: Returns -1 in the case of an invalid inode number.  Else, this
 *           reads length bytes starting from position offset in the file with
 *           inode and returning the number of bytes read and placed in the buffer.
 *           When the file is reached, 0 is returned.
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {
    int i;
    int n=0;
    uint32_t dentries_amount;  // number of dentries
    uint32_t data_block_number;  // the data block number to read from
    int8_t* data_block_addr;     // the address of the data block 
    uint32_t inode_amount;       // the number of inodes
    uint32_t rest_of_block;    // how many bytes left to read in the current data block
    uint32_t size_of_file;     
    int32_t num_bytes_read = 0;  // count of num bytes read
    uint32_t total_left_to_read;   // the total amount of bytes left to read
    dentries_amount = boot_block->num_dentries;
    inode_amount = boot_block->num_inodes; 
    dentry_t* curr_dentry = boot_block->dentries;   // 64 is the size of the statistics and reserved section of boot block
    if (inode >= boot_block->num_inodes) {
        return -1;
    }
    for (i = 0; i < dentries_amount; i++) {  // 63 is the max number of dentries
        curr_dentry += 1;
        if (curr_dentry->inode_num == inode) {
            inode_t* inode_struct =  (inode_t*)(boot_block + 1 + (curr_dentry->inode_num));// boot_block + BLOCK_SIZE * inode;  // get the correct inode block
            size_of_file = inode_struct->length;

            // if we want to start reading at a point that's past the file, just read nothing
            if (offset > size_of_file) {
                return 0;
            }
            while (offset > BLOCK_SIZE) {
                offset -= BLOCK_SIZE; 
                n += 1;   // n is the number of data blocks this stuff uses and is used as an index into the data block nums in the inode struct
            }

            data_block_number = inode_struct->data_blocks[n];  // get the data block number to access
            /* change from (data_block_number >= (boot_block->num_data_blocks)-1) to (data_block_number > (boot_block->num_data_blocks)-1)
             * during writing system call for checkpoint 3
             */
            if (data_block_number > (boot_block->num_data_blocks)-1) {
                return -1;
            }
            data_block_addr = (int8_t*)(boot_block + (1 + inode_amount + data_block_number));  // get the address of the data block we want to read
            data_block_addr += offset;   // get the address of the place in the data block we want to read from
            
            // set total left to read as diff of file size and starting point (to handle case where length would force you to read past the end of the file)
            total_left_to_read = size_of_file - offset;
            if (length < total_left_to_read) {           // if the length is less than this, just set it to length
                total_left_to_read = length;
            }

            while (total_left_to_read > 0) {   // while the length of the bytes we want to read is greatere than 0
                if (total_left_to_read < BLOCK_SIZE) {
                    rest_of_block = total_left_to_read;
                } else {
                    rest_of_block = BLOCK_SIZE - offset;    // calculate the amount of block that we want to read
                }
                if (offset > 0) {   // if this is not the first instance and offset is not 0, set it to 0 because from now on, we want to read from the beginning of a data block
                    offset = 0;
                }
                memcpy(buf, data_block_addr, rest_of_block); 
                num_bytes_read += rest_of_block;
                buf += rest_of_block;  // move along the buffer
                n+=1;   // get the next data block number
                data_block_number = inode_struct->data_blocks[n];  // get the next data block number
                data_block_addr = (int8_t*)(boot_block + (1 + inode_amount + data_block_number));  // get the address of the next data block
                total_left_to_read -= rest_of_block;  // subtract the amount of bytes we've read so far from the length
            }
            return num_bytes_read;
        }
    }
    return -1;
}

/* 
 * get_directory_info
 * Inputs: void* buf, int32_t nbytes
 * Return Value: length_of_file
 * Function: reads files filename by filename, including the "."
 */
int32_t get_directory_info(int32_t* type) {
    uint32_t this_inode_num;
    uint32_t this_file_length;
    if (index > boot_block->num_dentries) {
        return 0;
    } 
    read_dentry_by_index(index, d_dentry_ptr);

    this_inode_num = d_dentry_ptr->inode_num;
    inode_t* this_inode = (inode_t*)(boot_block + 1 + this_inode_num);
    this_file_length = this_inode->length;
    *type = d_dentry_ptr->file_type;

    return this_file_length;
}

/* 
 * directory_open
 * Inputs: const uint8_t* filename
 * Return Value: 0
 * Function: opens a directory file
 */
int32_t directory_open(const uint8_t* filename) {
    index = 0;    // at a new open, start reading from first directory again
    read_dentry_by_name(filename, d_dentry_ptr);
    return 0;
}

/* 
 * directory_close
 * Inputs: int32_t fd
 * Return Value: 0
 * Function: does nothing for now
 */
int32_t directory_close(int32_t fd) {
    return 0;
}

/* 
 * directory_read
 * Inputs: int32_t fd, void* buf, int32_t nbytes
 * Return Value: length_of_name
 * Function: reads files filename by filename, including the "."
 */
int32_t directory_read(int32_t fd, void* buf, int32_t nbytes) {
    int8_t thirthree_buf[FILE_NAME_SIZE+1];  // buffer of 33 chars to get 32 char string with null term char
    uint32_t length_of_name;
    if (index > boot_block->num_dentries) {  
        return 0;
    } 
    read_dentry_by_index(index, d_dentry_ptr);
    index += 1;

    memcpy(thirthree_buf, d_dentry_ptr->file_name, FILE_NAME_SIZE);
    thirthree_buf[FILE_NAME_SIZE] = '\0';
    length_of_name = strlen(thirthree_buf);  // copy it into a length 33 size buffer and set last char to null
    memcpy(buf, d_dentry_ptr->file_name, length_of_name); 
    // return the length of the name
    return length_of_name;  // all the files have been read already
}


/* 
 * directory_write
 * Inputs: int32_t fd, const void* buf, int32_t nbytes
 * Return Value: -1
 * Function: does nothing for now
 */
int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}

/* 
 * file_open
 * Inputs: const uint8_t* filename
 * Return Value: 0
 * Function: initializes any temporary structures and opens a file 
 */
// keep track of open files with pcb 
int32_t file_open(const uint8_t* filename) {
    // go through the fd array, find the first slot that is available -> give us that fd
    offset = 0; 
    read_dentry_by_name(filename, fs_dentry_ptr);
    // cur_pcb->fd_array[fd]->file_position = 0;
    return 0;  // fd
}

/* 
 * file_close
 * Inputs: int32_t fd
 * Return Value: 0
 * Function: undoes anything done in the file open function and closes a file
 */
int32_t file_close(int32_t fd) {
    return 0;
}

/* 
 * file_read
 * Inputs: int32_t fd, void* buf, int32_t nbytes
 * Return Value: number of bytes read
 * Function: reads nbytes bytes of data from file into buf
 */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes) {
    int32_t ret;
    ret = read_data(cur_pcb->fd_array[fd].inode, cur_pcb->fd_array[fd].file_position, buf, nbytes);
    // ret = read_data(fs_dentry_ptr->inode_num, cur_pcb->fd_array[fd].file_position, buf, nbytes);
    offset += ret; 
    cur_pcb->fd_array[fd].file_position += ret;
    return ret;
}


/* 
 * file_write
 * Inputs: int32_t fd, const void* buf, int32_t nbytes
 * Return Value: -1
 * Function: does nothing for now
 */
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}
