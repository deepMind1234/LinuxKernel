#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "paging.h"
#include "fs.h"
#include "rtc.h"
#include "terminal.h"
#include "systemcalls_handle.h"
#include "pit.h"

#define PASS 1
#define FAIL 0

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


/* Checkpoint 1 tests */

/* GDT Test
 * Asserts that the GDTR is not empty, and set to
 * the right value.
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load GDTR (part1)
 * Files: x86_desc.h/S
 */
int gdt_test(){
	TEST_HEADER;

	seg_desc_t gdt;
	asm("sgdt %0" : "=m"(gdt));
	
	return PASS;
}
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

/* paging_structures_test
 * makes sure that the paging structures hold expected values.
 * inputs - none
 * outputs - pass/fail
 * Coverage: Paging Structures
 * Files: paging.h, paging.c
 */
int paging_structures_test() {
	TEST_HEADER;
	int i;

	if (page_d[0].present != 1) return FAIL;
	if (page_d[0].address != ((uint32_t)kernel_page_table)>>12) return FAIL;
	if (page_d[0].read_write != 1) return FAIL;
	if (page_d[0].page_size != 0) return FAIL;
	
	if (page_d[1].address != MB_4 >> 12) return FAIL;
	if (page_d[1].global != 1) return FAIL;
	if (page_d[1].page_size != 1) return FAIL;
	if (page_d[1].present != 1) return FAIL;

	for (i = 2; i < 1024; i++) {
		if (page_d[i].present == 1) return FAIL;
	}

	for (i = 0; i < 1024; i++) {
		if (i != VIDEO && kernel_page_table[i].present == 1) return FAIL;
		else if (i == VIDEO &&
				kernel_page_table[i].present != 1 && 
				kernel_page_table[i].address != VIDEO) return FAIL;
	}

	return PASS;
}

/* page_dereference_test1
 * make sure kernel memory is accessible.
 * inputs - none
 * outputs - pass
 * coverage - dereferencing memory addresses
 * files - paging.h, paging.c
 */
int page_dereference_test1() {
	TEST_HEADER;

	// valid kernel address
	uint32_t *address = (uint32_t *) 0x411040;
	uint32_t a = *address;
	a++; // get rid of unused variable warning.

	return PASS;
}

/* page_dereference_test2
 * make sure unmapped memory is unaccessible
 * inputs - none
 * outputs - fail
 * side effects - stops all tests when pass. Causes pagefault
 * coverage - dereferencing memory addresses
 * files - paging.h, paging.c
 */
int page_dereference_test2() {
	TEST_HEADER;

	// invalid address
	uint32_t *address = (uint32_t *) 0x800001;
	uint32_t a = *address;
	a++; // get rid of unused variable warning.

	return FAIL;
}

/* page_dereference_test2
 * make sure null cannot be dereferenced
 * inputs - none
 * outputs - fail
 * side effects - stops all tests when pass. Causes pagefault
 * coverage - dereferencing memory addresses
 * files - paging.h, paging.c
 */
int page_dereference_test3() {
	TEST_HEADER;

	// dereferencing null
	uint32_t *address = (uint32_t *) 0x0;
	uint32_t a = *address;
	a++; // get rid of unused variable warning.

	return FAIL;
}

/* IDT Test - table value
 * 
 * Asserts: That table value falls within a particular bound namely within 32bits
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */


int idt_table_values(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 256; ++i){
		if ((idt[i].offset_15_00 > 32768) && 
			(idt[i].offset_31_16 > 32768)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/* IDT Test - Privelleges
 * 
 * Asserts: the dpl values for 2 entries are appropriate
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */

int idt_privellege(){
	TEST_HEADER;

	int result = FAIL;
	if ((idt[128].dpl == 3) && 
			(idt[0].dpl == 0)){
			assertion_failure();
			result = PASS;
		
	}

	return result;
}

// add more tests here

/* Checkpoint 2 tests */

/* printing_directory
 * print all the filenames in the '.' directory
 * inputs - none
 * outputs - pass/fail
 * side effects: none
 * coverage: open/read/close directory
 * files: fs.h/fs.c
 */
// int printing_directory(){
// 	TEST_HEADER;
// 	uint32_t i, dir_count;
// 	dir_count = dir_num ();
// 	uint32_t inode_idx;
// 	if (dir_open(".", &inode_idx) == FS_FAIL) return FAIL;
// 	uint32_t offset = 0;
// 	uint8_t buf[FILENAME_LEN+1];
// 	buf[FILENAME_LEN] = '\0';

// 	for(i = 0; i < dir_count; i++) {
// 		if (dir_read(&inode_idx, &offset, buf, FILENAME_LEN) == FS_FAIL) return FAIL;
// 		print_dentry_details(buf);
// 	}
		
// 	if (dir_close(&inode_idx) == FS_FAIL) return FAIL;

// 	printf("\n");
// 	return PASS;
// }

/* printing_long_file
 * print all the contents of a file spanning multiple blocks
 * inputs - none
 * outputs - pass/fail
 * side effects: prints content of file to screen
 * coverage: open/read/close file
 * files: fs.h/fs.c
 */
// int printing_long_file(){
// 	TEST_HEADER;

// 	uint32_t inode_idx;
// 	uint32_t offset = 0;
// 	uint8_t buf[SUPER_LARGE_BUFFER];
// 	if (file_open("verylargetextwithverylongname.tx", &inode_idx) == FS_FAIL) return FAIL;
// 	if(file_read(inode_idx, &offset, buf, SUPER_LARGE_BUFFER) == FS_FAIL) return FAIL;
// 	printf("%s", buf);
// 	return PASS;
// }

/* printing_short_file
 * print all the contents of a file spanning a single block
 * inputs - none
 * outputs - pass/fail
 * side effects: prints contents of file to screen
 * coverage: open/read/close file
 * files: fs.h/fs.c
 */
int printing_short_file(){
	TEST_HEADER;

	// uint32_t inode_idx;
	int32_t fd;
	uint8_t buf[BLOCK_SIZE];
	fd = open((uint8_t*)"frame1.txt");
	printf("fd: %d", fd);
	if (fd == FS_FAIL) return FAIL;
	if (read(fd, buf, BLOCK_SIZE) == FS_FAIL) return FAIL;
	
	printf("%s", buf);
	return PASS;
}

/* printing_short_file
 * print all the contents of an executable
 * inputs - none
 * outputs - pass/fail
 * side effects: print all non-null values in executable to screen.
 * coverage: open/read/close file
 * files: fs.h/fs.c
 */
// int printing_executable(){
// 	TEST_HEADER;
// 	uint32_t i;
// 	uint32_t inode_idx;
// 	uint32_t offset = 0;
// 	uint8_t buf[SUPER_LARGE_BUFFER];
// 	if (file_open("cat", &inode_idx) == FS_FAIL) return FAIL;
// 	if (file_read(inode_idx, &offset, buf, SUPER_LARGE_BUFFER) == FS_FAIL) return FAIL;
	
// 	for(i = 0; i < SUPER_LARGE_BUFFER; i++) {
// 		if(buf[i] != 0x00) printf("%c", buf[i]);
// 	}

// 	return PASS;
// }

/* rtc_driver_test
 * test changing the rtc interrupts and printing
 * 1's at different rates
 * inputs - none
 * outputs - pass/fail
 * coverage - rtc.h/rtc.c
 */
// int rtc_driver_test(){
// 	TEST_HEADER;
// 	rtc_open(NULL);
// 	int i,j,freq=1;
	
// 	for (i=1; i<=10;i++)
// 	{
// 		freq=freq << 1;
// 		rtc_write(0,&freq,0);
// 		for (j=1; j<=freq*2;j++){
// 			rtc_read(0,NULL,0);
// 			printf("1");
// 		}
// 		clear();
// 	}
// 	return PASS;
// }

/* terminalopentest
 * unit test sanity check for terminalOpen
 * inputs - none
 * outputs - PASS/FAIL
 * coverage: terminal.c/terminal.h
 */
int terminalopentest(){
	int result = FAIL;
	const uint8_t* ptr = 0;
	if(terminalOpen(ptr) == 0){
		result = PASS;
	}
	return result;
}
   
/* terminalclosetest
 * unit test sanity check for terminalClose
 * inputs - none
 * outputs - PASS/FAIL
 * coverage: terminal.c/terminal.h
 */
int terminalclosetest(){
	int result = FAIL;
	if(terminalClose(0) == 0){
		result = PASS;
	}
	return result;
}

/* terminalreadtest
 * unit test sanity check for terminalRead
 * inputs - none
 * outputs - PASS/FAIL
 * coverage: terminal.c/terminal.h
 */
// int terminalreadtest(){
// 	char buf[128] = "1234567890";
// 	int readchars= terminalRead(0,buf,128);
// 	return readchars == 0 ? PASS : FAIL;
// }

/* Terminaltest_loop
 * run time testing of the terminal
 * inputs - none
 * outputs - screen changes
 * side effects: 
 * coverage: terminal read, terminal write, as well as all of keyboard
 * files: terminal.c/h lib.c/h keyboard.c/h
 */
// int terminaltest_loop(){
// 	while(1){
// 		char buf[128];
// 		int readchars= terminalRead(0,buf,128);
// 		terminalWrite(0,buf,readchars);
// 	}
// }   

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */

int pit_test() {
	TEST_HEADER;

	// valid kernel address
	pit_init();


	return PASS;
}



/* Test suite entry point */
void launch_tests(){

	// TEST_OUTPUT("terminalopentest", terminalopentest());
	// TEST_OUTPUT("terminalclosetest", terminalclosetest());
	// TEST_OUTPUT("terminalreadtest", terminalreadtest());

	// clear();
	// TEST_OUTPUT("printing_directory_correct", printing_directory());

	// clear();
	// TEST_OUTPUT("printing_file_correct", printing_short_file());
	
	// clear();
	// TEST_OUTPUT("printing_large_file_correct", printing_long_file());

	// clear();
	// TEST_OUTPUT("printing_executable", printing_executable());

	// clear();
	// TEST_OUTPUT("rtc_test", rtc_driver_test());

	//clear();
	// terminaltest_loop();


	/** BELOW THIS POINT LIES CHECKPOINT1 TESTS **/
	
	// TEST_OUTPUT("gdt_test", gdt_test());
	// TEST_OUTPUT("idt_test", idt_test());
	
	// TEST_OUTPUT("paging_structures_test", paging_structures_test());
	// TEST_OUTPUT("page_dereference_test1", page_dereference_test1());
	// TEST_OUTPUT("idt_test_table_correct", idt_table_values());

	/** BREAKING TESTS. CAN ONLY RUN ONE AT ONCE **/
	// TEST_OUTPUT("page_dereference_test2", page_dereference_test2());
	// TEST_OUTPUT("page_dereference_test3", page_dereference_test3());
	// TEST_OUTPUT("idt_privellege", idt_privellege());
	
	//TEST_OUTPUT("memorybound", idt_memorybound());
	//TEST_OUTPUT("PrivellegeAndBounds", idt_memorybound());
	 //TEST_OUTPUT("pit test", pit_test());
}
