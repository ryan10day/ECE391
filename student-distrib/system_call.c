#include "system_call.h"
#include "file_system.h"
#include "x86_desc.h"
#include "lib.h"
#include "scheduler.h"

//uint32_t num_process = 0;
uint32_t cur_process_index = 0;
int first_process_flag = 0;
PCB_t pcb_array[6]; // the maximum number of process we can have is 6
int pcb_status[6] = {0,0,0,0,0,0};  // 1 or 0 to indicate which process is 'active'
PCB_t* cur_pcb;
int scheduling_array[3] = {-1,-1,-1};  // pid's for respective terminals

/* create four function table for different file type */
func_table_t directory_func_table = {directory_read, directory_write, directory_open, directory_close};
func_table_t rtc_func_table = {RTC_read, RTC_write, RTC_open, RTC_close};
func_table_t file_func_table = {file_read, file_write, file_open, file_close};
func_table_t stdin = {terminal_read, NULL, NULL, NULL};
func_table_t stdout = {NULL, terminal_write, NULL, NULL};
// func_table_t terminal_func_table = {terminal_read, terminal_write, terminal_open, terminal_close};
// Standard I/o should have separate func tables

PTE_t vmem_page_table[P_DIR_TABLE_SIZE] __attribute__((aligned (ALIGN_BOUNDARY)));

volatile int tar_disp_terminal = 1;   // terminal we want to display after switch
volatile int disp_terminal = 1;    // terminal being displayed
volatile int terminal_running = 1;    // terminal with the running process
volatile int target_terminal = 1;   // terminal we want to switch to via scheduling
int second_shell = 0;
int third_shell = 0;
int init_end = 0;

terminal_t terminal_1;
terminal_t terminal_2;
terminal_t terminal_3;
terminal_t* terminal_array[3];

void terminal_init ()
{
    terminal_array[0] = &terminal_1;
    terminal_array[1] = &terminal_2;
    terminal_array[2] = &terminal_3;
    terminal_1.terminal_num = 1;
    terminal_1.keyboard_buffer_index = 0;
    terminal_1.keyboard_flag = 0;
    terminal_1.cur_proc_idx = 0;
    terminal_1.first_process_flag = 0;
    terminal_1.command_idx = 0;
    terminal_1.screen_x = 0;
    terminal_1.screen_y = 0;
    terminal_1.video_mem = (char *)(TERMINAL1_VIDEO);

    terminal_2.terminal_num = 2;
    terminal_2.keyboard_buffer_index = 0;
    terminal_2.keyboard_flag = 0;
    terminal_2.cur_proc_idx = 1;
    terminal_2.first_process_flag = 0;
    terminal_2.command_idx = 0;
    terminal_2.screen_x = 0;
    terminal_2.screen_y = 0;
    terminal_2.video_mem = (char *)(TERMINAL2_VIDEO);

    terminal_3.terminal_num = 3;
    terminal_3.keyboard_buffer_index = 0;
    terminal_3.keyboard_flag = 0;
    terminal_3.cur_proc_idx = 2;
    terminal_3.first_process_flag = 0;
    terminal_3.command_idx = 0;
    terminal_3.screen_x = 0;
    terminal_3.screen_y = 0;
    terminal_3.video_mem = (char *)(TERMINAL3_VIDEO);

}

void switch_terminal(int prev_terminal, int next_terminal)
{   
    if (prev_terminal == next_terminal){
        return;
    }
    save_xy(disp_terminal);   // save screen position

    // load xb8000 contents into display terminal's page for later
    memcpy((uint8_t*)(VIDEO + disp_terminal * FOUR_KILA), (uint8_t*)(VIDEO), FOUR_KILA);
    // load xb8000 with contents of next terminal so we can see what's going on
    memcpy((uint8_t*)VIDEO, (uint8_t*)(VIDEO + next_terminal * FOUR_KILA), FOUR_KILA);
    disp_terminal = next_terminal;
    // get new screen pos' and refresh cursor pos
    load_xy(terminal_array[disp_terminal-1]->screen_x, terminal_array[disp_terminal-1]->screen_y);
    update_cursor_2(screen_x, screen_y);
}

/* 
 * flush_tlb
 * Inputs: 
 * Return Value: 
 * Function: Flushes the tlb
 */
void flush_tlb(){
    // from osdev
    asm volatile (
        "movl %%cr3, %%eax;"
        "movl %%eax, %%cr3;"      
        :                         // outputs
        :                         // inputs 
        : "eax"                   // clobbers
    );
}

/* 
 * map_pcb
 * Inputs: int num_process
 * Return Value: PCB_t*
 * Function: This function initializes a pcb for the currently running process
 */
PCB_t* map_pcb(int cur_process_index)
{
    /* 8Mb - (process index + 1) * 8kb to find top left pointer of pcb */
    PCB_t * pcb_ptr = (PCB_t *)(EIGHT_MEGA - (cur_process_index + 1) * EIGHT_KILA);
    // in stead of check first_process_flag check flag in each terminal
    // if (first_process_flag == 0)
    /* if we have first_process_flag == 0 it means we do not have the first process*/
    if (terminal_array[terminal_running - 1]->first_process_flag == 0)
    {
        pcb_ptr->parent_pid = -1;  
        pcb_ptr->current_pid = terminal_running - 1; 
        // first_process_flag = 1;
        terminal_array[terminal_running - 1]->first_process_flag = 1;
    }
    else
    {
        // calculate parent_pid and current_pid based on how many process we have 
        pcb_ptr->parent_pid = terminal_array[terminal_running-1]->cur_proc_idx;  
        pcb_ptr->current_pid = cur_process_index; 
    }
    // stdin in file directory
    pcb_ptr->fd_array[0].file_op_table_ptr = &stdin;
    pcb_ptr->fd_array[0].in_use = 1;
    // stdout in file directory
    pcb_ptr->fd_array[1].file_op_table_ptr = &stdout;
    pcb_ptr->fd_array[1].in_use = 1;
    return pcb_ptr;
}

/* 
 * func_table_ptr
 * Inputs: uint32_t file_type
 * Return Value: func_table_t*
 * Function: This function returns the function table pointer based on the file type
 */
func_table_t* func_table_ptr(uint32_t file_type)
{
    switch(file_type)
    {
        case 0:
            return &rtc_func_table;
        case 1:
            return &directory_func_table;
        case 2:
            return &file_func_table;
        default:
            return NULL;
    }
}


/* 
 * system_call_halt
 * Inputs: uint8_t status
 * Return Value: 0
 * Function: This halts a running process by restoring parent data and paging, closing 
 *           relevant fds and jumpting to the execute return spot in order to 
 *           return to the correct place in kernel space
 */
int32_t system_call_halt (uint8_t status)
{
    cli();
    /* restore parent data */
    int32_t temp_ebp;
    int32_t temp_esp;
    uint32_t exception_return = 256; // the function should return 256 if there is exxception
    temp_ebp = cur_pcb->current_ebp;
    temp_esp = cur_pcb->current_esp;
    // num_process -= 1;
    terminal_array[terminal_running - 1]->keyboard_flag = 0;
    pcb_status[cur_process_index] = 0;
    // modify this if statement consider different terminals 
    if (terminal_array[terminal_running-1]->cur_proc_idx == terminal_running-1 )
    {
        terminal_array[terminal_running-1]->first_process_flag = 0;
        system_call_execute((uint8_t* )"shell");
    }
    //clear FD array from 2 to 7(dynamically allocated)
    int i;
    for(i = 2; i<8; i++){
        cur_pcb->fd_array[i].in_use = 0;
    }
    
    cur_pcb = &pcb_array[cur_pcb->parent_pid];   // process done, now back to parent
    cur_process_index = cur_pcb->current_pid;
    terminal_array[terminal_running-1]->cur_proc_idx = cur_process_index;
    scheduling_array[terminal_running - 1] = cur_process_index;

    /* Restore parent paging */
    page_directory[VIRTUAL_PAGE_NUMBER].MEGA.present = 1;  // set PDE as valid
    page_directory[VIRTUAL_PAGE_NUMBER].MEGA.read_write = 1;  // make it read/writeable
    page_directory[VIRTUAL_PAGE_NUMBER].MEGA.page_size = 1;  // set as 1 so we know its a 4MB page
    page_directory[VIRTUAL_PAGE_NUMBER].MEGA.user_supervisor = 1;
    page_directory[VIRTUAL_PAGE_NUMBER].MEGA.base_address = (cur_process_index + 2);  // set addr to pid to put in correct 4MB slot in phys mem
    // flush the TLB
    flush_tlb();
    // temp_ebp = cur_pcb->current_ebp;
    // temp_esp = cur_pcb->current_esp;
    /* Close any relevant FDs */
    // minus 4 to prevent page fault 
    tss.ss0 = KERNEL_DS;
    tss.esp0 = EIGHT_MEGA - cur_pcb->current_pid * EIGHT_KILA - 4;
    // random number we choose for status when we have exceptions
    if (status == 63)
    {
        sti();
        /* jump to label in execute */
        asm volatile (
            "movl %0, %%ebp;"
            "movl %1, %%esp;"
            "movl %2, %%eax;"
            "jmp label_halt;"
            :                  // outputs
            : "r" (temp_ebp), "r"(temp_esp), "r"(exception_return)                // inputs 
            : "eax"                 // clobbers
    );
    }
    sti();
    /* jump to label in execute */
    asm volatile (
        "movl %0, %%ebp;"
        "movl %1, %%esp;"
        "movl %2, %%eax;"
        "jmp label_halt;"
        :                  // outputs
        : "r" (temp_ebp), "r"(temp_esp), "r"((uint32_t)status)                // inputs 
        : "eax"                 // clobbers
    );
    return 0;
}


/* 
 * system_call_execute
 * Inputs: const uint8_t* command
 * Return Value: 0
 * Function: This executes the command in kernel.c or shell.c, maps the appropriate paging,
 *           reads the correct file data into the allocated page, maps a pcb, and performs
 *           a context switch to hand control over to user space. 
 */
int32_t system_call_execute (const uint8_t* command)
{
    cli();
    int32_t temp;
    int32_t nbytes_read;
    dentry_t d_dentry;
    dentry_t* temp_dentry = &d_dentry;
    uint32_t type;
    uint32_t execute_start_addr;
    unsigned char magic_constant_buf[40]; // buffer used to get first 40 char in the file
    uint8_t test_addr[4]; // bufer used to get first insruction address (24 -27 bytes)
    int i;
    int six_pcb_flag = 1;
    // terminal_t* terminal_ptr;
    // unsigned char temp_command_buf[NUM_COMMAND][KEYBOARD_BUF_SIZE] = command_buffer;
    // int i;
    // /*use for cursor up */
    // if (command_idx < NUM_COMMAND)
    // {
    //     memset(command_buffer[command_idx], 0,MAX_COMMAND_LEN); 
    //     memcpy(command_buffer[command_idx], command, strlen((char*)command));
    //     command_idx += 1;
    // }
    // else if (command_idx == NUM_COMMAND)
    // {
    //     for(i = 0; i<NUM_COMMAND-1; i++)
    //     {
    //         memset(command_buffer[i], 0, MAX_COMMAND_LEN); 
    //         memcpy(command_buffer[i], command_buffer[i+1], MAX_COMMAND_LEN);
    //     }
    //     memset(command_buffer[NUM_COMMAND - 1], 0, MAX_COMMAND_LEN); 
    //     memcpy(command_buffer[NUM_COMMAND - 1], command, strlen((char*)command));
    // }

    /* check whether we have a null pointer */
    if (command == NULL)
    {
        sti();
        return -1;
    }
    /* delete spaces before command */
    while(strlen((char*)command) !=  0)
    {
        if (command[0] == ' ')
        {
            command += 1;
        }
        else
        {
            break;
        }
        
    }
    /* delete spaces after last char in command/file */
    int COMMAND_LEN = strlen((char*)command);
    uint8_t exename[MAX_COMMAND_LEN];   //string for the excecutable
    uint8_t arg[MAX_COMMAND_LEN];       //string for the arguments(only have space for one)
    int cursor = 0;

    memset(exename, 0,MAX_COMMAND_LEN); //initialize all the strings to 0
    memset(arg, 0,MAX_COMMAND_LEN);
    while(cursor < COMMAND_LEN){                         //copy first part (command) to exename
        if(command[cursor] != ' ' && command[cursor] != '\n'){
            exename[cursor] = command[cursor];
        }else{
            exename[cursor + 1] = '\n';
            cursor ++;
            break;
        }
    cursor ++;
    }
    /* get rid of spaces in between the command and file */
    while(cursor < COMMAND_LEN){
        if(command[cursor] == ' '){
            cursor ++;
        }else{
            break;
        }
    }


    int offset = cursor;

    while(cursor < COMMAND_LEN){                         //copy the arguemnts to arg
        if(command[cursor] != '\n'){
            arg[cursor - offset] = command[cursor];
        }else{
            arg[cursor + 1] = '\n';
            cursor ++;
            break;
        }
    cursor ++;
    }

    /* Check for executable */
    /* Check whether we have file for given command*/
    temp = read_dentry_by_name(exename, temp_dentry);
    if (temp == -1)
    {
        sti();
        return -1;
    }
    /* Check type for the file */
    type = temp_dentry->file_type;
    if (type == 0 || type == 1)
    {
        sti();
        return -1;
    }

    /* Checks ELF magic constant */
    /* read first 40 bytes in the file*/
    nbytes_read = read_data(temp_dentry->inode_num, 0, magic_constant_buf, 40);
    if(magic_constant_buf[0] != 0x7f || magic_constant_buf[1] != 0x45 || magic_constant_buf[2] != 0x4c || magic_constant_buf[3] != 0x46)
    {
        sti();
        return -1;
    }
    /* check whether we have more than six process*/
    for(i=0; i<6; i++)
    {
        if(pcb_status[i] == 0)
        {
            pcb_status[i] = 1;
            cur_process_index = i;
            six_pcb_flag = 0;
            break;
        }
    }
    // check whether we have maximum of processes which is 6
    if (six_pcb_flag == 1)
    {
        printf("REACHED MAX NUMBER OF PROCESSES!\n");
        sti();
        return -1;
    }
    scheduling_array[terminal_running - 1] = cur_process_index;
    /* setting up paging */
    // the user shell process address is 128 MB - 132 MB (thirty second page in memory)
    page_directory[VIRTUAL_PAGE_NUMBER].MEGA.present = 1;  // set PDE as valid
    page_directory[VIRTUAL_PAGE_NUMBER].MEGA.read_write = 1;  // make it read/writeable
    page_directory[VIRTUAL_PAGE_NUMBER].MEGA.page_size = 1;  // set as 1 so we know its a 4MB page
    page_directory[VIRTUAL_PAGE_NUMBER].MEGA.user_supervisor = 1;
    page_directory[VIRTUAL_PAGE_NUMBER].MEGA.base_address = (cur_process_index + 2);  // set addr to pid to put in correct 4MB slot in phys mem


    // flush the TLB
    flush_tlb();

    /* Load file into memory */
    // virtual memory & paging map to physical memory 
    
    /* get first instruction address (24-27 bytes) */
    read_data(temp_dentry->inode_num, 24, (uint8_t*)test_addr, 4);
    execute_start_addr = *((uint32_t*)test_addr);

    /* Copy file contents to the correct location */
    nbytes_read = read_data(temp_dentry->inode_num, 0, (uint8_t*)PROGRAM_IMAGE_START, PROGRAM_IMAGE_SIZE);


    /* Create PCB (process control block ) / open FDs */
    // create a pcb structure and a function to map pcb to memory 
    // in child's PCB, store parent task's PCB pointer
    /* global variable num_process to keep tracking how many processes are running */
    // num_process += 1;
    // cur_process_index = terminal_array[disp_terminal - 1]->cur_proc_idx;
    cur_pcb = map_pcb(cur_process_index);
    // cur_process_index = cur_pcb->current_pid;
    terminal_array[terminal_running-1]->cur_proc_idx = cur_process_index;

    asm volatile (
        "movl %%esp, %0;"
        "movl %%ebp, %1;" 
        : "=r"(cur_pcb->current_esp), "=r"(cur_pcb->current_ebp)   // outputs
        :                         // inputs 
        : "eax"                   // clobbers
    );

    cur_pcb->schedule_ebp = cur_pcb->current_ebp;
    cur_pcb->schedule_esp = cur_pcb->current_esp;

    //pcb_array[cur_process_index] = *cur_pcb;
    memcpy(&(pcb_array[cur_process_index]),cur_pcb,sizeof(PCB_t));
    // terminal_ptr = terminal_array[terminal_running - 1];
    // terminal_ptr->pcb_array[terminal_ptr->cur_proc_idx] = *cur_pcb;
    // terminal_array[terminal_running - 1]->cur_proc_idx = cur_process_index;
    
    memcpy(cur_pcb->arg,arg,MAX_COMMAND_LEN);       // copy local variable into the PCB

    
    /* Prepare for Context Switch */
    /* 
     * Note that when you start a new process,
     * just before you switch to that process and start executing its user-level code, 
     * you must alter the TSS entry to contain
     * its new kernel-mode stack pointer
     */
    /* Push IRET context to kernel stack */
    // modify ss0 and esp0 during context switch 
    tss.ss0 = KERNEL_DS;
    // tss.esp0 = 0x8400000;
    tss.esp0 = EIGHT_MEGA - cur_pcb->current_pid * EIGHT_KILA - 4;
    sti();
    asm volatile (
        "movl %0, %%eax;"

        "pushl $0x002B;"    // USER_DS defined in x86_desc.h
        "pushl %1;"      // 132MB-4B 
        "pushfl;"
        "pushl $0x0023;"    // USER_CS defined in x86_desc.h 
        "pushl %%eax;"     // starting address of user program
        "iret;"              // from kernel space to user space
        // label used when called system_call_halt
        ".globl label_halt;"
        "label_halt:;"
        "leave;"
        "ret;"
        :                         // outputs
        : "r" (execute_start_addr), "r" (PROGRAM_IMAGE_END)    // inputs  r - load into any available register
        : "eax"               // clobbers
    );
    // context_switch(execute_start_addr);

    /*return*/
    return 0;

}

/* 
 * system_call_read
 * Inputs: int32_t fd, void* buf, int32_t nbytes
 * Return Value: number of bytes read or -1
 * Function: This function calls the appropriate read function based on the file type
 */
int32_t system_call_read (int32_t fd, void* buf, int32_t nbytes)
{
    int32_t return_value;
    /* Check if fd is in bounds (0,7) */
    if (fd < 0 || fd > 7)
    {
        return -1;
    }
    /* check if fd is in use or not */
    if(cur_pcb->fd_array[fd].in_use ==0){
        return -1;
    }
    /* check if we have a valid function */
    if(cur_pcb->fd_array[fd].file_op_table_ptr->r == NULL )
    {
        return -1;
    }
    /* call corresponding function based on fd */
    return_value = cur_pcb->fd_array[fd].file_op_table_ptr->r(fd, buf, nbytes);
    return return_value;
}



/* 
 * system_call_write
 * Inputs: int32_t fd, void* buf, int32_t nbytes
 * Return Value: number of bytes written or -1
 * Function: This function calls the appropriate write function based on the file type
 */
int32_t system_call_write (int32_t fd, const void* buf, int32_t nbytes)
{
    int32_t return_value;
    /* Check if fd is in bounds (0,7) */ 
    if (fd < 0 || fd > 7)
    {
        return -1;
    }
    /* check if fd is in use or not */
    if(cur_pcb->fd_array[fd].in_use ==0){
        return -1;
    }
    /* check if we have a valid function */
    if(cur_pcb->fd_array[fd].file_op_table_ptr->w == NULL )
    {
        return -1;
    }
    /* call corresponding function based on fd */
    return_value = cur_pcb->fd_array[fd].file_op_table_ptr->w(fd, buf, nbytes);
    return return_value;
}


/* 
 * system_call_open
 * Inputs: const uint8_t* filename
 * Return Value: 
 * Function: This function calls the appropriate open function based on the file type
 */
int32_t system_call_open (const uint8_t* filename)
{   
    if(filename == NULL){                           //check invalid input
        return -1;
    }
    if(strlen((char*)filename) ==0){
        return -1;
    }           
    int32_t temp;
    dentry_t d_dentry;
    dentry_t* temp_dentry = &d_dentry;
    temp = read_dentry_by_name(filename, temp_dentry);      //open dentry for filename
    if (temp == -1)
    {
        return -1;
    }
    // if(func_table_ptr(temp_dentry->file_type)[2](filename)<0){
    //     return -1;
    // }
    int i;
    for (i = 2; i< 8; i++){                                 //find empty entry in fd array
        if (cur_pcb->fd_array[i].in_use == 0){      
            cur_pcb->fd_array[i].file_op_table_ptr = func_table_ptr(temp_dentry->file_type);        //populate op table
            if(0!=cur_pcb->fd_array[i].file_op_table_ptr->o(filename)){                             //if cant open test, return -1
                return -1;
            }
            cur_pcb->fd_array[i].in_use = 1;            //mark as used
            cur_pcb->fd_array[i].file_position = 0;
            cur_pcb->fd_array[i].inode = temp_dentry->inode_num;
            memcpy(&(pcb_array[cur_process_index]),cur_pcb,sizeof(PCB_t));
            return i;
        }
    }
    return -1;
}

/* 
 * system_call_close
 * Inputs: int32_t fd
 * Return Value: 0 or -1
 * Function: This function calls the appropriate close function based on the file type
 */
int32_t system_call_close (int32_t fd)
{
    int32_t return_value;
    // Check invalid input
    if (fd < 2 || fd > 7)
    {
        return -1;
    }
    /* check if fd is in use or not */
    if(cur_pcb->fd_array[fd].in_use ==0){
        return -1;
    }
    /* check if we have a valid function */
    if(cur_pcb->fd_array[fd].file_op_table_ptr->c == NULL )
    {
        return -1;
    }
    /* call corresponding function based on fd */
    return_value = cur_pcb->fd_array[fd].file_op_table_ptr->c(fd);
    cur_pcb->fd_array[fd].in_use = 0;                                   // mark the entry as empty
    return return_value;
}


/* 
 * system_call_getargs
 * Inputs: uin8_t* buf, int32_t nbytes
 * Return Value: 0 or -1
 * Function: This function reads the program's command line arguments into a user-level
 *          buffer
 */
int32_t system_call_getargs (uint8_t* buf, int32_t nbytes)
{
    /* check if valid buf */
    if (buf == NULL){
        return -1;
    }
    /* check if length of arg does not exceed available bytes */
    if(strlen((char*)cur_pcb->arg) > nbytes){
        return -1;
    }
    /* check if the length of arg equal to 0 */
    if(strlen((char*)cur_pcb->arg) == 0){
        return -1;
    }
    /* load the command and stuff into buffer */
    int cursor;
    for (cursor = 0; cursor < nbytes - 1; cursor ++){
        buf[cursor] = cur_pcb->arg[cursor];
    }
    buf[nbytes-1] = '\n';
    return 0;
}

/* 
 * system_call_vidmap
 * Inputs: uint8_t** screen_start
 * Return Value: 0 or -1
 * Function: This function maps the text-mode video memory into user space at a 
 *           pre-set virtual address.
 */
int32_t system_call_vidmap (uint8_t** screen_start)
{
    /* check if we can deref screen_start */
    if(screen_start ==NULL){
        return -1;
    }
    /* make sure that screen start is within 128-132MB */
    if(ONE_TWENTY_EIGHT + FOUR_MEGA <= (int)screen_start || (int)screen_start < ONE_TWENTY_EIGHT){
        return -1;
    }
        /* do page mapping for video memory with newly allocated page table */
        page_directory[33].KILA.present = 1;  // set PDE as valid
        page_directory[33].KILA.read_write = 1;  // make it read/writeable
        page_directory[33].KILA.page_size = 0;  // set as 0 so we know its a 4KB page
        page_directory[33].KILA.user_supervisor = 1;
        page_directory[33].KILA.base_address = (int)vmem_page_table >> 12;  // set addr to pid to put in correct 4MB slot in phys mem, shift by 12 to get offset
        int pt_index =  VIDEO_MEMORY_ADDR>>12;   // shift by 12 as above
        vmem_page_table[pt_index].TABLE.present = 1;
        vmem_page_table[pt_index].TABLE.read_write = 1;
        vmem_page_table[pt_index].TABLE.user_supervisor = 1;
        vmem_page_table[pt_index].TABLE.base_address = (int)video_mem>>12;   // shift by 12 as above

        flush_tlb();
    
    /* set derefed screen start so single pointer is to right below the user-level program */
    *screen_start = (uint8_t*)ONE_TWENTY_EIGHT + FOUR_MEGA + VIDEO_MEMORY_ADDR;
    return 0;

}


int32_t system_call_set_handler (int32_t signum, void* handler_address)
{
    return -1;
}

int32_t system_call_sigreturn (void)
{
    return -1;
}
