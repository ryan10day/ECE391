#include "types.h"
#pragma once

#define P_DIR_TABLE_SIZE  1024       // size of page_dir/page_tables
#define ALIGN_BOUNDARY    4096       // make it alignable by 4096
#define KERNEL_ADDR       0x400000   // kernel page starts at 4MB
#define VIDEO_MEMORY_ADDR 0x0b8000   // video memory address retrieved from lib.c


// structs for a page directory 4-kb table entry and a 4-mb page entry
typedef struct PDE {
    union {
        uint32_t val;
        struct {  // for a 4-kb table entry
            uint32_t present             :1;
            uint32_t read_write          :1;
            uint32_t user_supervisor     :1;
            uint32_t write_through       :1;
            uint32_t cache_disabled      :1;
            uint32_t accessed            :1;
            uint32_t set_to_0            :1;
            uint32_t page_size           :1;
            uint32_t global_page         :1;
            uint32_t available           :3;
            uint32_t base_address       :20;
        } KILA __attribute__((packed));
        struct {  // for a 4-mb page entry
            uint32_t present               :1;
            uint32_t read_write            :1;
            uint32_t user_supervisor       :1;
            uint32_t write_through         :1;
            uint32_t cache_disabled        :1;
            uint32_t accessed              :1;
            uint32_t dirty                 :1;
            uint32_t page_size             :1;
            uint32_t global_page           :1;
            uint32_t available             :3;
            uint32_t page_table_attr_idx   :1;
            uint32_t reserved             :9;
            uint32_t base_address         :10;
        } MEGA __attribute__ ((packed));
    };
} PDE_t;


// struct for a 4-kb page entry in a page table
typedef struct PTE {
    union {
        uint32_t val;
        struct {
            uint8_t present             :1;
            uint8_t read_write          :1;
            uint8_t user_supervisor     :1;
            uint8_t write_through       :1;
            uint8_t cache_disabled      :1;
            uint8_t accessed            :1;
            uint8_t dirty               :1;
            uint8_t page_table_attr_idx :1;
            uint8_t global_page         :1;
            uint8_t available           :3;
            uint32_t base_address       :20;
        } TABLE __attribute__ ((packed));
    };
} PTE_t;


extern PDE_t page_directory[P_DIR_TABLE_SIZE] __attribute__((aligned (ALIGN_BOUNDARY)));
extern PTE_t page_table[P_DIR_TABLE_SIZE] __attribute__((aligned (ALIGN_BOUNDARY)));


void init_paging();
