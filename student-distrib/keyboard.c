#include "keyboard.h"
#include "lib.h"
#include "Terminal_Driver.h"
#include "system_call.h"
/* created array based on scan code set 1 from https://wiki.osdev.org/Keyboard */
/* the first one represent without shift, second one represent with shift */
/* \t for tab */
/* 1: without shift or capslock */
/* 2: with capslock but without shift */
/* 3: with shift but without capslock */
/* 4: with both shift and capslock */
unsigned char scan_code_set1[92][4] = {
    {0, 0, 0, 0},         {0, 0, 0, 0},         {'1', '1', '!', '!'}, {'2', '2', '@', '@'},
    {'3', '3', '#', '#'}, {'4', '4', '$', '$'}, {'5', '5', '%', '%'}, {'6', '6', '^', '^'},
    {'7', '7', '&', '&'}, {'8', '8', '*', '*'}, {'9', '9', '(', '('}, {'0', '0', ')', ')'},
    {'-', '-', '_', '_'}, {'=', '=', '+', '+'}, {'\b', '\b', '\b', '\b'}, {'\t', '\t', '\t', '\t'},
    {'q', 'Q', 'Q', 'q'}, {'w', 'W', 'W', 'w'}, {'e', 'E', 'E', 'e'}, {'r', 'R', 'R', 'r'},
    {'t', 'T', 'T', 't'}, {'y', 'Y', 'Y', 'y'}, {'u', 'U', 'U', 'u'}, {'i', 'I', 'I', 'i'},
    {'o', 'O', 'O', 'o'}, {'p', 'P', 'P', 'p'}, {'[', '[', '{', '{'}, {']', ']', '}', '}'},
    {'\n', '\n', '\n', '\n'}, {0, 0, 0, 0},     {'a', 'A', 'A', 'a'}, {'s', 'S', 'S', 's'},
    {'d', 'D', 'D', 'd'}, {'f', 'F', 'F', 'f'}, {'g', 'G', 'G', 'g'}, {'h', 'H', 'H', 'h'},
    {'j', 'J', 'J', 'j'}, {'k', 'K', 'K', 'k'}, {'l', 'L', 'L', 'l'}, {';', ';', ':', ':'},
    {'\'', '\'', '\"', '\"'}, {'`', '`', '~', '~'}, {0, 0, 0, 0},     {'\\', '\\', '|', '|'},
    {'z', 'Z', 'Z', 'z'}, {'x', 'X', 'X', 'x'}, {'c', 'C', 'C', 'c'}, {'v', 'V', 'V', 'v'},
    {'b', 'B', 'B', 'b'}, {'n', 'N', 'N', 'n'}, {'m', 'M', 'M', 'm'}, {',', ',', '<', '<'},
    {'.', '.', '>', '>'}, {'/', '/', '?', '?'}, {0, 0, 0, 0},         {0, 0, 0, 0},
    {0, 0, 0, 0},         {' ', ' ', ' ', ' '}, {0, 0, 0, 0},         {0, 0, 0, 0},  
    {0, 0, 0, 0},         {0, 0, 0, 0},         {0, 0, 0, 0},         {0, 0, 0, 0},  
    {0, 0, 0, 0},         {0, 0, 0, 0},         {0, 0, 0, 0},         {0, 0, 0, 0}, 
    {0, 0, 0, 0},         {0, 0, 0, 0},         {0, 0, 0, 0},         {0, 0, 0, 0},
    {0, 0, 0, 0},         {0, 0, 0, 0},         {0, 0, 0, 0},         {0, 0, 0, 0},
    {0, 0, 0, 0},         {0, 0, 0, 0},         {0, 0, 0, 0},         {0, 0, 0, 0},
    {0, 0, 0, 0},         {0, 0, 0, 0},         {0, 0, 0, 0},         {0, 0, 0, 0},
    {0, 0, 0, 0},         {0, 0, 0, 0},         {0, 0, 0, 0},         {0, 0, 0, 0},  
    {0, 0, 0, 0},         {0, 0, 0, 0},         {0, 0, 0, 0},         {0, 0, 0, 0} 
};

/* created array based on scan code set 2 from https://wiki.osdev.org/Keyboard */
/* the first one represent without shift, second one represent with shift */
unsigned char scan_code_set2[132][2] = {
    {0, 0},     {0, 0},     {0, 0},     {0, 0},
    {0, 0},     {0, 0},     {0, 0},     {0, 0},
    {0, 0},     {0, 0},     {0, 0},     {0, 0},
    {0, 0},     {0, 0},     {0, 0},     {0, 0},
    {0, 0},     {0, 0},     {0, 0},     {0, 0},
    {0, 0},     {'q', 'Q'}, {'1', '!'}, {0, 0},
    {0, 0},     {0, 0},     {'z', 'Z'}, {'s', 'S'},
    {'a', 'A'}, {'w', 'W'}, {'2', '@'}, {0, 0},
    {0, 0},     {'c', 'C'}, {'x', 'X'}, {'d', 'D'},
    {'e', 'E'}, {'4', '$'}, {'3', '#'}, {0, 0},
    {0, 0},     {'0', '0'}, {'v', 'V'}, {'f', 'F'},
    {'t', 'T'}, {'r', 'R'}, {'5', '%'}, {0, 0},
    {0, 0},     {'n', 'N'}, {'b', 'B'}, {'h', 'H'},
    {'g', 'G'}, {'y', 'Y'}, {'6', '^'}, {0, 0},
    {0, 0},     {0, 0},     {'m', 'M'}, {'j', 'J'},
    {'u', 'U'}, {'7', '&'}, {'8', '*'}, {0, 0},
    {0, 0},     {',', '<'}, {'k', 'K'}, {'i', 'I'},
    {'o', 'O'}, {'0', ')'}, {'9', '('}, {0, 0},
    {0, 0},     {'.', '>'}, {'/', '?'}, {'l', 'L'},
    {';', ':'}, {'p', 'P'}, {'-', '_'}, {0, 0},
    {0, 0},     {0, 0},     {'\'', '\"'}, {0, 0},
    {'[', '{'}, {'=', '+'}, {0, 0},     {0, 0},
    {0, 0},     {0, 0},     {0, 0},     {0, 0},
    {0, 0},     {'\\', '|'}, {0, 0},    {0, 0},
    {0, 0},     {0, 0},     {0, 0},     {0, 0},
    {0, 0},     {0, 0},     {0, 0},     {0, 0},
    {0, 0},     {'1', 0},   {0, 0},     {'4', 0},
    {'7', 0},   {0, 0},     {0, 0},     {0, 0},
    {'0', 0},   {'.', 0},   {'2', 0},   {'5', 0},
    {'6', 0},   {'8', 0},   {0, 0},     {0, 0},
    {0, 0},     {'+', 0},   {'3', 0},   {'-', 0},
    {'*', 0},   {'9', 0},   {0, 0},     {0, 0},
    {0, 0},     {0, 0},     {0, 0},     {0, 0}
};

unsigned int left_shift_flag = 0;          // flag for holding left shift  
unsigned int right_shift_flag = 0;         // flag for holding right shift
unsigned int Capslock_flag = 0;            // flag for pressed caps lock
unsigned int left_ctrl_flag = 0;           // flag for holding left ctrl 
unsigned int alt_flag = 0;                 // flag for holding alt

/* void keyboard_init();
 * Inputs: none
 * Return Value: none
 * Function: init the keyboard by call enable_irq in i8259.c */
void keyboard_init()
{
    enable_irq(KEYBOARD_IRQ);
}

/* void keyboard_get_char();
 * Inputs: none
 * Return Value: none
 * Function: print the char typed on keyboard */
void keyboard_get_char()
{
    cli();
    // send_eoi(KEYBOARD_IRQ);
    char* temp = video_mem;
    video_mem = (char*)VIDEO;
    // video_mem = terminal_array[disp_terminal-1]->video_mem;
    unsigned char scan_code;
    int i;
    /* get scan code using inb function*/
    scan_code = inb(KEYBOARD_DATA);
    /* if we detect user pressed left shift (scan code 0x2A) */
    if (scan_code == 0x2A)
    {
        left_shift_flag = 1;
    }
    /* if we detect user pressed right shift (scan mode 0x36) */
    if (scan_code == 0x36)
    {
        right_shift_flag = 1;
    }
    /* if we detect user released left shift (scan code 0xAA) */
    if (scan_code == 0xAA)
    {
        left_shift_flag = 0;
    }
    /* if we detect user released right shift (scan code 0xB6) */
    if (scan_code == 0xB6)
    {
        right_shift_flag = 0;
    }
    /* if we detect user pressed CapsLock Ox3A in scan code set 1 (scan code 0x3A) */
    if (scan_code == 0x3A)
    {
        if(Capslock_flag == 0)
        {
            Capslock_flag = 1;
        }
        else
        {
            Capslock_flag = 0;
        }
    }

    /* if we detect pressed left contorl (scan code 0x1D) */
    if (scan_code == 0x1D)
    {
        left_ctrl_flag = 1;
    }
    /* if we detect released left control (scan code 0x9D) */
    if (scan_code == 0x9D)
    {
        left_ctrl_flag = 0;
    }

    /* if we detect pressed alt (scan code 0x38) */
    // for now we use a 0x1E cause we don't have alt
    if (scan_code == 0x38)
    {
        alt_flag = 1;
    }
    /* if we detect released alt (scan code 0xB8) */
    // for now we use a 0x9E cause we don't have alt
    if (scan_code == 0xB8)
    {
        alt_flag = 0;
    }

    /* if we detect cursor up pressed (scan code 0x48 )*/
    // if (scan_code == 0x48)
    // {
    //     if (terminal_array[disp_terminal - 1]->command_idx == 0)
    //     {
    //         send_eoi(KEYBOARD_IRQ);
    //         sti();
    //         return;
    //     }
    //     for (i=0; i<terminal_array[disp_terminal - 1]->keyboard_buffer_index; i++)
    //     {
    //         putc('\b');
    //     }
    //     terminal_array[disp_terminal - 1]->command_idx -=  1;
    //     terminal_array[disp_terminal - 1]->keyboard_buffer_index = strlen((char*)terminal_array[disp_terminal - 1]->command_buffer[terminal_array[disp_terminal - 1]->command_idx]);
    //     terminal_write (1, terminal_array[disp_terminal - 1]->command_buffer[terminal_array[disp_terminal - 1]->command_idx], terminal_array[disp_terminal - 1]->keyboard_buffer_index);
    //     // printf((char*)command_buffer[command_idx]);
    //     memcpy(terminal_array[disp_terminal - 1]->keyboard_buffer, terminal_array[disp_terminal - 1]->command_buffer[terminal_array[disp_terminal - 1]->command_idx], terminal_array[disp_terminal - 1]->keyboard_buffer_index);
    //     send_eoi(KEYBOARD_IRQ);
    //     return;
    // }

    /* check whether we have scan_code represents release using 0x80 mask*/
    if ((scan_code & 0x80) == 0)
    {
        if (alt_flag == 1)
        {
            if (scan_code == 0x3b)  // 3b, 3c, 3d scancodes for f1, f2, f3
            {
                send_eoi(KEYBOARD_IRQ);
                tar_disp_terminal = 1;    // if alt f1 is pressed, we want to display term1
                video_mem = temp;      // restore video_mem from b8000 to avoid artifacts
                sti();
                return;
            }
            if (scan_code == 0x3c)
            {
                send_eoi(KEYBOARD_IRQ);
                tar_disp_terminal = 2;   // if alt f2 is pressed, we want to display term2
                video_mem = temp;
                sti();
                return;
            }
            if (scan_code == 0x3d)
            {
                send_eoi(KEYBOARD_IRQ);
                tar_disp_terminal = 3;   // if alt f3 is pressed, we want to display term3
                video_mem = temp;
                sti();
                return;
            }
        }
        /* if we have pressed left control */
        if (left_ctrl_flag == 1)
        {
            /* if we have a pressed l (0x26) in the scancode set 1*/
            if (scan_code == 0x26)
            {
                /* call the clear function in lib.c */
                int temp_term = terminal_running;
                terminal_running = disp_terminal;
                clear();
                printf("391OS> ");
                for(i = 0; i<terminal_array[disp_terminal - 1]->keyboard_buffer_index; i++)
                {
                     putc(terminal_array[disp_terminal - 1]->keyboard_buffer[i]);
                }
                terminal_running = temp_term;
                send_eoi(KEYBOARD_IRQ);
                video_mem = temp;
                sti();
                return;
            }
            /* if we have a pressed c (0x2E) in the scancode set 1 */
            // if (scan_code == 0x2e)
            // {
            //     send_eoi(KEYBOARD_IRQ);
            //     /* halt from current process */
            //     int temp = target_terminal;
            //     target_terminal = disp_terminal;
            //     scheduler();
            //     target_terminal = temp;
            //     scheduler();

            //     system_call_halt(0);
            //     sti();
            //     return;
            // }
        }
        if (terminal_array[disp_terminal - 1]->keyboard_flag == 1)
        {
            send_eoi(KEYBOARD_IRQ);
            video_mem = temp;
            sti();
            return;
        }
        char_typed = scan_code_set1[scan_code][0];
        if (char_typed == 0)
        {
            send_eoi(KEYBOARD_IRQ);
            video_mem = temp;
            sti();
            return;
        }
        /* if we detected a pressed enter */
        if (char_typed == '\n')
        {
            /* set the flag and store \n in the keyboard buffer */
            terminal_array[disp_terminal - 1]->keyboard_flag = 1;
            terminal_array[disp_terminal - 1]->keyboard_buffer[terminal_array[disp_terminal - 1]->keyboard_buffer_index] = '\n';
            terminal_array[disp_terminal - 1]->keyboard_buffer_index++;
            int temp_term = terminal_running;
            terminal_running = disp_terminal;
            putc(char_typed);
            terminal_running = temp_term;
            send_eoi(KEYBOARD_IRQ);
            video_mem = temp;
            sti();
            return;
        }
        /* if we detected a pressed backspace */
        else if (char_typed == '\b')
        {
            /* if we have char in keyboard buffer, decrement number of char in buffer*/
            if(terminal_array[disp_terminal - 1]->keyboard_buffer_index != 0)
            {
            terminal_array[disp_terminal - 1]->keyboard_buffer_index --;
            int temp_term = terminal_running;
            terminal_running = disp_terminal;
            putc(char_typed);
            terminal_running = temp_term;
            }
            send_eoi(KEYBOARD_IRQ);
            video_mem = temp;
            sti();
            return;
        }
        /* if we have maximum number of keyboard buffer */
        if (terminal_array[disp_terminal - 1]->keyboard_buffer_index >= KEYBOARD_BUF_SIZE - 1)
        {
            send_eoi(KEYBOARD_IRQ);
            video_mem = temp;
            sti();
            return;
        }
        if (Capslock_flag == 1)
        {
            if (left_shift_flag == 0 && right_shift_flag == 0)
            {
                /* output with capslock but without shift */
                char_typed = scan_code_set1[scan_code][1];
                terminal_array[disp_terminal - 1]->keyboard_buffer[terminal_array[disp_terminal - 1]->keyboard_buffer_index] = char_typed;
                terminal_array[disp_terminal - 1]->keyboard_buffer_index += 1;
                int temp_term = terminal_running;
                terminal_running = disp_terminal;
                putc(char_typed);
                terminal_running = temp_term;
            }
            else
            {
                /* output with capslock and shift*/
                char_typed = scan_code_set1[scan_code][3];
                terminal_array[disp_terminal - 1]->keyboard_buffer[terminal_array[disp_terminal - 1]->keyboard_buffer_index] = char_typed;
                terminal_array[disp_terminal - 1]->keyboard_buffer_index += 1;
                int temp_term = terminal_running;
                terminal_running = disp_terminal;
                putc(char_typed);
                terminal_running = temp_term;
            }
        }
        else
        {
            if (left_shift_flag == 0 && right_shift_flag == 0)
            {
                /* output with shift but without capslock */
                char_typed = scan_code_set1[scan_code][0];
                terminal_array[disp_terminal - 1]->keyboard_buffer[terminal_array[disp_terminal - 1]->keyboard_buffer_index] = char_typed;
                terminal_array[disp_terminal - 1]->keyboard_buffer_index += 1;
                int temp_term = terminal_running;
                terminal_running = disp_terminal;
                putc(char_typed);
                terminal_running = temp_term;
            }
            else
            {
                /* output without shift and capslock */
                char_typed = scan_code_set1[scan_code][2];
                terminal_array[disp_terminal - 1]->keyboard_buffer[terminal_array[disp_terminal - 1]->keyboard_buffer_index] = char_typed;
                terminal_array[disp_terminal - 1]->keyboard_buffer_index += 1;
                int temp_term = terminal_running;
                terminal_running = disp_terminal;
                putc(char_typed);
                terminal_running = temp_term;
            }
        }
    }
    /* send end-interrupt signal */
    send_eoi(KEYBOARD_IRQ);
    video_mem = temp;
    // video_mem = terminal_array[terminal_running-1]->video_mem;
    sti();
}

