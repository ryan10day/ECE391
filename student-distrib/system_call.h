#include "types.h"
#include "file_system.h"
#include "Terminal_Driver.h"
#include "RTC.h"
#include "paging.h"

#define PROGRAM_IMAGE_SIZE  0x03B8000
#define PROGRAM_IMAGE_START 0x08048000
#define PROGRAM_IMAGE_END 0x083ffffc    // 132MB-4B 
#define ONE_TWENTY_EIGHT 0x08000000
#define FOUR_MEGA 0x400000
#define EIGHT_MEGA 0x800000
#define EIGHT_KILA 0x2000
#define VIRTUAL_PAGE_NUMBER 32
#define MAX_COMMAND_LEN 128

typedef int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
typedef int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
typedef int32_t (*open)(const uint8_t* filename);
typedef int32_t (*close)(int32_t fd);

extern PTE_t vmem_page_table[P_DIR_TABLE_SIZE] __attribute__((aligned (ALIGN_BOUNDARY)));

/* function table struct */
typedef struct func_table {
    read r;
    write w;
    open o;
    close c;
} func_table_t;

typedef struct file_descriptor_t {
    func_table_t* file_op_table_ptr;
    uint32_t inode;
    uint32_t file_position;
    uint32_t in_use;
} file_descriptor_t;

typedef struct PCB {
    file_descriptor_t fd_array[8];      // :     128;
    int32_t parent_pid;                 // :     4;
    int32_t current_pid;                // :     4;
    int32_t schedule_esp;                 // :     4;
    int32_t current_esp;
    int32_t schedule_ebp;
    int32_t current_ebp;
    uint8_t arg[128];
} PCB_t;

typedef struct terminal {
    int terminal_num;
    unsigned char keyboard_buffer[KEYBOARD_BUF_SIZE];
    volatile unsigned int keyboard_buffer_index;
    volatile int keyboard_flag;
    PCB_t pcb_array[4]; 
    int cur_proc_idx;
    int first_process_flag;
    unsigned char command_buffer[NUM_COMMAND][KEYBOARD_BUF_SIZE];
    int command_idx;
    int screen_x;
    int screen_y;
    char* video_mem;
} terminal_t;

extern int scheduling_array[3];
extern PCB_t pcb_array[6];
extern terminal_t terminal_1;
extern terminal_t terminal_2;
extern terminal_t terminal_3;
extern terminal_t* terminal_array[3];
extern volatile int tar_disp_terminal;
// extern volatile int active_terminal;
extern volatile int total_proc;
extern volatile int disp_terminal;
extern volatile int terminal_running;
extern volatile int target_terminal;
extern int second_shell;
extern int third_shell;
extern int init_end;
extern uint32_t cur_process_index;

extern PCB_t* cur_pcb;

void flush_tlb();
// void context_switch(uint32_t execute_start_addr);
func_table_t* func_table_ptr(uint32_t file_type);

PCB_t* map_pcb(int current_pid);
void terminal_init ();
void switch_terminal(int prev_terminal, int next_terminal);
extern int32_t system_call_halt (uint8_t status);
extern int32_t system_call_execute (const uint8_t* command);
extern int32_t system_call_read (int32_t fd, void* buf, int32_t nbytes);
extern int32_t system_call_write (int32_t fd, const void* buf, int32_t nbytes);
extern int32_t system_call_open (const uint8_t* filename);
extern int32_t system_call_close (int32_t fd);
extern int32_t system_call_getargs (uint8_t* buf, int32_t nbytes);
extern int32_t system_call_vidmap (uint8_t** screen_start);
extern int32_t system_call_set_handler (int32_t signum, void* handler_address);
extern int32_t system_call_sigreturn (void);
