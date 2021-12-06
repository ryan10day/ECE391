#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "RTC.h"
#include "keyboard.h"
#include "Terminal_Driver.h"
#include "file_system.h"
#define PASS 1
#define FAIL 0

#define VIDEO_START_ADDRESS  0x0b8000
#define VIDEO_END_ADDRESS    0x0b8fff
#define KERNEL_START_ADDRESS 0x400000
#define KERNEL_END_ADDRESS   0x7fffff


/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}

extern unsigned char keyboard_buffer[128];
/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

// add more tests here


//////////////////////////////  PAGING TESTS START HERE /////////////////////////////
/* page_test_k_start
 * 
 * Asserts that there is a valid dereference at the start of allocated kernel memory
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: paging functionality
 * Files: paging.c
 */
int page_test_k_start() {
	TEST_HEADER;
	int* k_start = (int*)KERNEL_START_ADDRESS;
	int val;
	val = *k_start;
	return PASS;
}

/* page_test_v_start
 * 
 * Asserts that there is a valid dereference at the start of allocated video memory
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: paging functionality
 * Files: paging.c
 */
int page_test_v_start() {
	TEST_HEADER;	
	int* v_start = (int*)VIDEO_START_ADDRESS;
	int val;
	val = *v_start;
	return PASS;
}

/* page_test_k_end
 * 
 * Asserts that there is a valid dereference at the end of allocated kernel memory
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: paging functionality
 * Files: paging.c
 */
int page_test_k_end() {
	TEST_HEADER;
	uint8_t* k_end = (uint8_t*)(KERNEL_END_ADDRESS);
	uint8_t val;
	val = *k_end;
	return PASS;
}

/* page_test_v_end
 * 
 * Asserts that there is a valid dereference at the end of allocated video memory
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: paging functionality
 * Files: paging.c
 */
int page_test_v_end() {
	TEST_HEADER;	
	uint8_t* v_end = (uint8_t*)(VIDEO_END_ADDRESS);
	uint8_t val;
	val = *v_end;
	return PASS;
}

/* page_test_v_middle
 * 
 * Asserts that there is a valid dereference in the middle of allocated video memory
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: paging functionality
 * Files: paging.c
 */
int page_test_v_middle() {
	TEST_HEADER;	
	int* v_end = (int*)(VIDEO_END_ADDRESS - 1000);
	int val;
	val = *v_end;
	return PASS;
}

/* page_test_k_middle
 * 
 * Asserts that there is a valid dereference at the end of allocated kernel memory
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: paging functionality
 * Files: paging.c
 */
int page_test_k_middle() {
	TEST_HEADER;	
	int* v_end = (int*)(KERNEL_END_ADDRESS - 0x200000);
	int val;
	val = *v_end;
	return PASS;
}

/* page_test_k_oob_before
 * 
 * Asserts that there is a page fault when trying to dereference 'out of bounds' pointer
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: paging functionality
 * Files: paging.c
 */
int page_test_k_oob_before() {
	TEST_HEADER;
	int* k_before = (int*)(KERNEL_START_ADDRESS-1);
	int val;
	if (k_before == NULL) {
		return FAIL;
	}
	val = *k_before;
	return PASS;
}

/* page_test_v_oob_before
 * 
 * Asserts that there is a page fault when trying to dereference 'out of bounds' pointer
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: paging functionality
 * Files: paging.c
 */
int page_test_v_oob_before() {
	TEST_HEADER;
	int* v_before = (int*)(VIDEO_START_ADDRESS-1);
	int val;
	if (v_before == NULL) {
		return FAIL;
	}
	val = *v_before;
	return PASS;
}

/* page_test_k_oob_after
 * 
 * Asserts that there is a page fault when trying to dereference 'out of bounds' pointer
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: paging functionality
 * Files: paging.c
 */
int page_test_k_oob_after() {
	TEST_HEADER;
	int* k_after = (int*)(KERNEL_END_ADDRESS+1);
	int val;
	if (k_after == NULL) {
		return FAIL;
	}
	val = *k_after;
	return PASS;
}

/* page_test_v_oob_after
 * 
 * Asserts that there is a page fault when trying to dereference 'out of bounds' pointer
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: paging functionality
 * Files: paging.c
 */
int page_test_v_oob_after() {
	TEST_HEADER;
	int* v_after = (int*)(VIDEO_END_ADDRESS+1);
	int val;
	if (v_after == NULL) {
		return FAIL;
	}
	val = *v_after;
	return PASS;
}

/* rtc_test
 * 
 * Test whether rtc int is called
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: rtc int call
 * Files: RTC.c
 Side effect: spams 43k hz per second
 */
// int rtc_test() {
// 	TEST_HEADER;
// 	while(char_typed != '['){}
// 	clear();
// 	RTC_init();
// 	// clear();
// 	return PASS;
// }
// /* div_zero_test
//  * 
//  * Test whether div by zero exception is called
//  * Inputs: None
//  * Outputs: PASS/FAIL
//  * Side Effects: None
//  * Coverage: div by zero error
//  * Files: idt.c
//  Side effect: infinite loop
//  use: 0 key triggers exception
// 	  c key continues
//  */
// int div_zero_test() {
// 	TEST_HEADER;
// 	while(char_typed != '0'){
// 		if(char_typed == 'c'){
// 			return PASS;
// 		}
// 	}
// 	int a = 0;
// 	int b = 3/a;
// 	printf("%d",b);
// 	return PASS;
// }

// /* deref_null_test
//  * 
//  * Test whether div by page fault is called
//  * Inputs: None
//  * Outputs: PASS/FAIL
//  * Side Effects: None
//  * Coverage: page fault error
//  * Files: idt.c
//  Side effect: infinite loop
//  use: 9 key triggers exception
// 	  d key continues
//  */
// int deref_null_test() {
// 	TEST_HEADER;
// 	while(char_typed != '9'){
// 		if(char_typed == 'd'){
// 			return PASS;
// 		}
// 	}

// 	int * a = NULL;
// 	*a = 10;
// 	printf("%d",a);
// 	return PASS;
// }
/* Checkpoint 2 tests */

/* rtc_test2
 * 
 * runs the rtc test at various intervals
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: rtc functionality
 * Files: RTC.c
 */
int rtc_test2() {
	TEST_HEADER;
	uint32_t i,j;
	uint8_t fd = 1;
	RTC_open(&fd);		
	for (i = 2; i<=1024; i<<=1){			//loop through the frequencies from 2 to 1024(max) hz.
		for (j=0;j<32;j++){					//print only 32 character
			RTC_read(fd, &i, 4);			//pass intended frequency into the RTC_read
			printf("#");
		}
		printf("\n");
	}
	RTC_close(fd);
	return PASS;
}

/* terminal_test2
 * 
 * runs the terminal program
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Terminal program
 * Files: Terminal_Driver.c, keyboard.c
 */
int terminal_test() {
    uint32_t read_return;
    unsigned char buf[128];				//terminal buffer
	uint8_t fd = 1;
	terminal_open(&fd);
    while(1)							//loop between read and write
    {
        read_return = terminal_read (fd, buf, 128);
        terminal_write(fd, buf, read_return);
    }
	terminal_close (fd);
	return PASS;
}

/* read_file_test
 * 
 * Reads the desired file based on a file name
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: reading file capability
 * Files: file_system.c
 Side effect: spams 43k hz per second
 */
int read_file_test() {
	TEST_HEADER;
	clear();
	uint8_t fname[32] = "verylargetextwithverylongname.tx";
	// uint8_t fname[32] = "frame0.txt";
	// uint8_t fname[32] = "cat";
	uint8_t buf1[10000];
	uint32_t nbytes1;
	int i;
	int temp;
	temp = file_open(fname);
	// printf("return  value: %d \n", temp);
	nbytes1 = file_read(2, buf1, 10000);       // random fd index is 2, 10000 is a very generous read length
	for (i = 0; i < nbytes1; i++) {
		if (buf1[i] != '\0') {
			putc(buf1[i]);
		}
	}
	//printf("number of bytes read: %d \n", nbytes);
	file_close(2);
	printf("%s", fname);
	return PASS;
}

/* double_read_file_test
 * 
 * Reads the desired file based on a file name
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: reading file capability
 * Files: file_system.c
 Side effect: spams 43k hz per second
 */
int double_read_file_test() {
	TEST_HEADER;
	clear();
	uint8_t fname[32] = "verylargetextwithverylongname.tx";
	// uint8_t fname[32] = "frame0.txt";
	// uint8_t fname[32] = "cat";
	uint8_t buf1[4800];
	uint8_t buf2[1000];
	uint32_t nbytes1;
	uint32_t nbytes2;
	int i;
	int j;
	int temp;
	temp = file_open(fname);
	// printf("return  value: %d \n", temp);
	nbytes1 = file_read(2, buf1, 4800);       // random fd index is 2, 10000 is a very generous read length
	for (i = 0; i < nbytes1; i++) {
		if (buf1[i] != '\0') {
			putc(buf1[i]);
		}
	}
	nbytes2 = file_read(2, buf2, 1000);
	for (j = 0; j < nbytes2; j++) {
		if (buf2[j] != '\0') {
			putc(buf2[j]);
		}
	}
	//printf("number of bytes read: %d \n", nbytes);
	file_close(2);
	printf("%s", fname);
	return PASS;
}

/* read_directory_test
 * 
 * Demonstrates the ability to read file names
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: reading directory capability
 * Files: file_system.c
 Side effect: displays file names and their sizes
 */
int read_directory_test() {
	TEST_HEADER;
	clear();
	uint8_t* fname = (uint8_t*)".";
	uint8_t buf[32];   // size of file name
	uint32_t name_length;	//length of file name
	uint32_t offset;		//used in print formating
	int32_t type;			//type of file
	int32_t file_length;	//length of file in bytes
	int8_t length_buf[11];	//length of file in byte in string
	int temp;
	int i;
	int j;
	temp = directory_open(fname);
	for (i = 0; i < 17; i++) {    // the number of directory entries
		file_length = get_directory_info(&type);
		name_length = directory_read(2, buf, sizeof(buf));  // random fd index is 2
		printf("file_name: ");
		offset = 32-name_length;	
		for (j = 0; j < 32; j++) {			//total amount of character used in file name	
			if(j<offset){
				printf(" ");				//for printing file name, first print offset amount of space
			}else{
			if (buf[j-offset] != '\0') {
				putc(buf[j-offset]);		//print file name
			}
			}

		}
		
		printf(", file_type: %d", type);				//print file type
		itoa((uint32_t)file_length, length_buf, 10);
		printf(", file_size:");
		for(j = 0; j<(8-strlen(length_buf)); j++){		//8 character dedicated to file size
			printf(" ");
		}
		puts(length_buf);
		printf("\n");
	}
	// printf("File name: %s      file name length: %d     ", buf, name_length);
	return PASS;
}

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	clear();
	// launch your tests here

	// Checkpoint 1
	// TEST_OUTPUT("idt_test", idt_test());
	// TEST_OUTPUT("exception_div_0_test", div_zero_test());
	// TEST_OUTPUT("deref_null_test", deref_null_test());
	// TEST_OUTPUT("idt_test", idt_test());
	// TEST_OUTPUT("page_test_k_start", page_test_k_start());
	// TEST_OUTPUT("page_test_v_start", page_test_v_start());
	// TEST_OUTPUT("page_test_k_middle", page_test_k_middle());
	// TEST_OUTPUT("page_test_v_middle", page_test_v_middle());
	// TEST_OUTPUT("page_test_k_end", page_test_k_end());
	// TEST_OUTPUT("page_test_v_end", page_test_v_end());
	// TEST_OUTPUT("RTC_test", rtc_test());
	// TEST_OUTPUT("page_test_k_oob_before", page_test_k_oob_before());
	// TEST_OUTPUT("page_test_k_oob_after", page_test_k_oob_after());
	// TEST_OUTPUT("page_test_v_oob_before", page_test_v_oob_before());
	// TEST_OUTPUT("page_test_v_oob_after", page_test_v_oob_after());

	//Checkpoint 2
	// TEST_OUTPUT("RTC_test", rtc_test2());
	// TEST_OUTPUT("terminal_test", terminal_test());
	// TEST_OUTPUT("read_file_test", read_file_test());
	// TEST_OUTPUT("double_read_file_test", double_read_file_test());
	// TEST_OUTPUT("read_directory_test", read_directory_test());


}
