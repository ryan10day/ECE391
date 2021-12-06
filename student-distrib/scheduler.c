#include "scheduler.h"
#include "system_call.h"
#include "paging.h"
#include "lib.h"

/* void scheduler()
 * Inputs: none
 * Return Value: void
 * Function: pass control to next terminal determined by round robin
             algorithm, handle the switching of terminals  */
void scheduler()
{
    PCB_t* prev_pcb;
    PCB_t* next_pcb;
    int32_t temp_ebp;
    int32_t temp_esp;
    int temp;
    int ebp;
    int esp;

    // if(scheduling_array[terminal_running-1] == -1)
    // {
    //     return;
    // }
    // if(scheduling_array[target_terminal-1] == -1)
    // {
    //     return;
    // }

    cli();
    if(tar_disp_terminal != disp_terminal){
        switch_terminal(disp_terminal,tar_disp_terminal);
    }

    if(terminal_array[0]->first_process_flag == 0)  // execute first shell if no terminal has any
    {
        system_call_execute((uint8_t* )"shell");
        sti();
        return;
    }

    // at t3 to t1, note down what is getting moved into schedule_ebp/esp ( ^^ they should match  )
    // load curr stack info into 'previous' pcb
    prev_pcb = &pcb_array[scheduling_array[terminal_running-1]];
    asm volatile (
        // "pushal;"
        "movl %%esp, %0;"
        "movl %%ebp, %1;" 
        : "=r"(prev_pcb->schedule_esp), "=r"(prev_pcb->schedule_ebp)   // outputs
        :                         // inputs 
        : "eax"                 // clobbers
    );

    // terminal_running = (terminal_running % 3) + 1;
    terminal_running = target_terminal;    
    
    if(disp_terminal != terminal_running)
    {
        // repoint video_mem to terminal vid page so background terminal can update 
        video_mem = terminal_array[terminal_running-1]->video_mem;

        // int pt_index = (int)video_mem>>12;   // shift by 12 as above
        // vmem_page_table[pt_index].TABLE.base_address = (int)video_mem>>12;   // shift by 12 as above
        // flush_tlb();
        
    } else{
        video_mem = (char *)VIDEO;   // otherwise it should still be xb8000
    }

    if (scheduling_array[1] == -1) {   // if there is no process in the second term, get a shell in there
        save_xy(terminal_running);
        clear();
        ebp = EIGHT_MEGA - 1 * EIGHT_KILA - 4;  // right spot in kernel stack stack (1 since that is pid, 4 to avoid page fault)
        esp = ebp;
        // make ebp, esp vars so that control given to correct process
        asm volatile (
            "movl %0, %%ebp;"
            "movl %1, %%esp;"
            // "popal;"
            :                  // outputs
            : "r" (ebp), "r"(esp)                // inputs 
            : "eax"                 // clobbers
        );
        
        system_call_execute((uint8_t* )"shell");
        sti();
        return;
    }

    if (scheduling_array[2] == -1) {  // if no process in the third term, get a shell there
        init_end = 1;
        save_xy(terminal_running);
        clear();
        ebp = EIGHT_MEGA - 2 * EIGHT_KILA - 4;  // 2 is the pid, subtract 4 to avoid page fault
        esp = ebp;
        asm volatile (
            "movl %0, %%ebp;"
            "movl %1, %%esp;"
            // "popal;"
            :                  // outputs
            : "r" (ebp), "r"(esp)                // inputs 
            : "eax"                 // clobbers
        );
        
        system_call_execute((uint8_t* )"shell");
        sti();
        return;
    }

    // if(terminal_array[terminal_running-1]->first_process_flag == 0)
    // {
    //     //switch_terminal(terminal_running, target_terminal);
    //     if(terminal_running == 3)
    //     {
    //         init_end = 1;
    //     }
    //     clear();

    //     ebp = EIGHT_MEGA - 1 * EIGHT_KILA - 4;
    //     esp = ebp;
    //     asm volatile (
    //         "movl %0, %%ebp;"
    //         "movl %1, %%esp;"
    //         // "popal;"
    //         :                  // outputs
    //         : "r" (ebp), "r"(esp)                // inputs 
    //         : "eax"                 // clobbers
    //     );

    //     system_call_execute((uint8_t* )"shell");
    // }

    if(init_end == 1)
    {
        init_end = 0;
        save_xy(3);
    }

    next_pcb = &pcb_array[scheduling_array[terminal_running-1]];


        int pt_index =  VIDEO_MEMORY_ADDR>>12;   // shift by 12 as above
        vmem_page_table[pt_index].TABLE.base_address = (int)video_mem>>12;   // set vidmap paging for fishy; shift by 12 as above
        flush_tlb();
        
    page_directory[VIRTUAL_PAGE_NUMBER].MEGA.base_address = (scheduling_array[terminal_running-1] + 2);  // set addr to pid to put in correct 4MB slot in phys mem
    // flush the TLB
    flush_tlb();

    cur_process_index = scheduling_array[terminal_running-1];   
    cur_pcb = &pcb_array[cur_process_index];

    tss.ss0 = KERNEL_DS;
    tss.esp0 = EIGHT_MEGA - scheduling_array[terminal_running-1] * EIGHT_KILA - 4;
    temp_ebp = next_pcb->schedule_ebp;
    temp_esp = next_pcb->schedule_esp;
    
    asm volatile (
        "movl %0, %%ebp;"
        "movl %1, %%esp;"
        // "popal;"
        :                  // outputs
        : "r" (temp_ebp), "r"(temp_esp)                // inputs 
        : "eax"                 // clobbers
    );

    temp = 0;
    sti();
}
