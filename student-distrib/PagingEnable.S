.text
.globl loadPageDirectory, enablePaging

# load page_directory to cr3
# inputs - pointer to page_directory
# outputs - none
# side effects - copies address of page_directory to cr3
loadPageDirectory:
pushl %ebp
movl %esp, %ebp
movl 8(%ebp), %eax
movl %eax, %cr3
leave
ret

# enable paging by setting the cr0 register
# inputs - none
# outputs - none
# side effects - turns on paging.
enablePaging:
pushl %ebp
movl %esp, %ebp

movl %cr4, %eax
orl $0x00000010, %eax
movl %eax, %cr4

movl %cr0, %eax
orl $0x80000000, %eax
movl %eax, %cr0


leave
ret
