#ifndef TESTS_H
#define TESTS_H

// test launcher

void launch_tests();
int page_test_k_start();
int page_test_k_end();
int page_test_v_start();
int page_test_v_end();
int page_test_k_middle();
int page_test_v_middle();
int page_test_k_oob_before();
int page_test_k_oob_after();
int page_test_v_oob_before();
int page_test_v_oob_after();

	int idt_test();
	int div_zero_test();
	int deref_null_test();
	int idt_test();
	int rtc_test();


	//Checkpoint 2
	int rtc_test2();
	int terminal_test();
	int read_file_test();
	int double_read_file_test();
	int read_directory_test();

#endif /* TESTS_H */
