#include "paging.h"


PDE_t page_directory[P_DIR_TABLE_SIZE] __attribute__((aligned (ALIGN_BOUNDARY)));
PTE_t page_table[P_DIR_TABLE_SIZE] __attribute__((aligned (ALIGN_BOUNDARY)));
/* 
 * void init_paging()
 * Inputs: none
 * Return Value: none
 * Function: initializes paging by setting page directory and page table entry values
 *           to support the allocation of a 4-MB page for kernel memory and a 4-KB
 *           page for video memory. 
 */
void init_paging() {  
    int i;
    for (i=0; i < P_DIR_TABLE_SIZE; i++) {   // set default values
        page_directory[i].val = 0;  // clear the struct values
        page_table[i].val = 0;
        // page_directory[i].MEGA.read_write = 1;   // for this mp, default for all pages is r/w enabled
        // page_directory[i].KILA.read_write = 1;
        // page_table[i].TABLE.read_write = 1;
    }

    // the kernel address is 4 MB - 8 MB (second page in memory)
    page_directory[1].MEGA.present = 1;  // set PDE as valid
    page_directory[1].MEGA.read_write = 1;
    page_directory[1].MEGA.page_size = 1;  // set as 1 so we know its a 4MB page
    page_directory[1].MEGA.base_address = 1;  // set addr to 1

    // set the page table that video memory is in
    page_directory[0].KILA.present = 1;
    page_directory[0].KILA.read_write = 1;
    page_directory[0].KILA.base_address = (int)page_table >> 12; // page_table ; shift by 12 to account for alignment

    // video memory address is 0xb8000
    // first 4mb are divided up into 1024 4kb pages (one is for video memory)
    // 184 4kb values are needed to get to 0xb8000 (0xb8 = 184)
    int pt_index = VIDEO_MEMORY_ADDR>>12;   // get 0xb8, the correct page index (184)
    // set the 4kb page that contains video memory
    page_table[pt_index].TABLE.present = 1;
    page_table[pt_index].TABLE.read_write = 1;
    page_table[pt_index].TABLE.base_address = pt_index;  // address is index from video memory

    // set the 4kb page that contains video memory for terminal 1
    page_table[pt_index + 1].TABLE.present = 1;
    page_table[pt_index + 1].TABLE.read_write = 1;
    page_table[pt_index + 1].TABLE.base_address = pt_index + 1;  // address is index from video memory

    // set the 4kb page that contains video memory for terminal 2
    page_table[pt_index + 2].TABLE.present = 1;
    page_table[pt_index + 2].TABLE.read_write = 1;
    page_table[pt_index + 2].TABLE.base_address = pt_index + 2;  // address is index from video memory

    // set the 4kb page that contains video memory for terminal 3
    page_table[pt_index + 3].TABLE.present = 1;
    page_table[pt_index + 3].TABLE.read_write = 1;
    page_table[pt_index + 3].TABLE.base_address = pt_index + 3;  // address is index from video memory


    // took this directly from OSDev
    // use %% for actual register
    asm volatile (
        "movl %0, %%eax;"
        "movl %%eax, %%cr3;"      // move the page directory base to cr3
        "movl %%cr4, %%eax;"
        "orl $0x00000010, %%eax;"   // set the PSE bit in CR4 high to enable 4MB pages
        "movl %%eax, %%cr4;"
        "movl %%cr0, %%eax;"
        "orl $0x80000001, %%eax;"    // set the PE bit and the page-enable bit high in CR0
        "movl %%eax, %%cr0;"
        :                         // outputs
        : "r" (page_directory)    // inputs  r - load into any available register
        : "eax"                   // clobbers
    );
}
