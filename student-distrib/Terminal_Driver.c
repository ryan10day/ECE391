#include "Terminal_Driver.h"
#include "lib.h"
#include "system_call.h"
#include "paging.h"

// volatile unsigned int keyboard_buffer_index = 0;
// volatile int keyboard_flag = 0;
// unsigned char command_buffer[NUM_COMMAND][KEYBOARD_BUF_SIZE];
// int command_idx = 0;


/* int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes)
 * Inputs: int32_t fd, void* buf, int32_t nbytes
 * Return Value: int32_t num bytes read
 * Function:  reads FROM the keyboard buffer into buf, return number
            of bytes read, similar to stdin */
int32_t terminal_read (int32_t fd, void* buf, int32_t nbytes)
{
    int32_t command_length;
    // uint8_t temp_buf[6] = "shell\n";
    /* return -1 if we have a empty buffer */
    if (buf == NULL)
    {
        return -1;
    }
    /* return -1 if we have a empty keyboard buffer */
    if (terminal_array[terminal_running - 1]->keyboard_buffer == NULL)
    {
        return -1;
    }
    terminal_array[terminal_running - 1]->keyboard_buffer_index = 0;
    terminal_array[terminal_running - 1]->keyboard_flag = 0;
    /* keep checking whether we have /n */
    while(1)
    {
        // (t[b].enter == 0 || d != b) ---> t[b].enter == 1 && d == b
        cli();
        if (terminal_running == disp_terminal && terminal_array[terminal_running - 1]->keyboard_flag == 1)
        {
            break;
        }
        sti();
        // command_length = keyboard_buffer_index;
        // if(keyboard_buffer[command_length] == '\n')
        // {
        //     break;
        // }
    }
    command_length = terminal_array[disp_terminal - 1]->keyboard_buffer_index;
    /* check whether command_length smaller or nbytes smaller */
    if (command_length < nbytes)
    {
        memcpy(buf, terminal_array[disp_terminal - 1]->keyboard_buffer, command_length);
        terminal_array[disp_terminal - 1]->keyboard_buffer_index = 0;
        terminal_array[disp_terminal - 1]->keyboard_flag = 0;
        sti();
        return command_length;
    }
    /* if we have smaller nbytes than number of char in keyboard buffer */
    else
    {
        memcpy(buf, terminal_array[disp_terminal - 1]->keyboard_buffer, nbytes);
        terminal_array[disp_terminal - 1]->keyboard_buffer_index = 0;
        terminal_array[disp_terminal - 1]->keyboard_flag = 0;
        sti();
        return nbytes;
    }
}


/* int32_t terminal_write(int32_t fd, void* buf, int32_t nbytes)
 * Inputs: int32_t fd, void* buf, int32_t nbytes
 * Return Value: int32_t num bytes written
 * Function:  writes TO the screen from buf, return number of bytes
            written or -1, similar to stdout */
int32_t terminal_write (int32_t fd, const void* buf, int32_t nbytes)
{
    int i;
    /* return -1 if we have a empty buffer */
    if (buf == NULL)
    {
        return -1;
    }
    // memcpy(temp_buf, buf, nbytes);
    for (i=0; i<nbytes; i++)
    {
        putc(((char*)buf)[i]);
    }
    return nbytes;
}

/* int32_t terminal_open(const uint8_t* filename)
 * Inputs: const uint8_t* filename
 * Return Value: 0
 * Function: initializes terminal stuff (or nothing), return 0 */
int32_t terminal_open (const uint8_t* filename)
{
    return 0;
}


/* int32_t terminal_close(int32_t fd)
 * Inputs: int32_t fd
 * Return Value: 0
 * Function: clears any terminal specific variables (or do nothing), return 0 */
int32_t terminal_close (int32_t fd)
{
    return 0;
}
