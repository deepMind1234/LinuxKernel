.text
.globl irq0,irq1,irq2,irq3,irq4,irq5,irq6,irq7,irq8,irq9,irq10,irq11,irq12,irq13,irq14,irq15,irq16,irq17,irq18,irq19,irq20,irq32,irq33,irq40,irq128

.data
.globl pagefault
pagefault:
        .long 0
.globl syscall_ret
syscall_ret:
        .long 0xFFFFFFFF


systemcalls:
    .long 0xFF, halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn

irq0:
pushal
pushfl
cli
push $0
call do_irq
addl $4, %ESP
popfl
popal
iret 
  

irq1:
pushal
pushfl
cli
push $1
call do_irq
addl $4, %ESP
popfl
popal
iret 

irq2:
pushal
pushfl
cli
push $2
call do_irq
addl $4, %ESP
popfl
popal
iret 

irq3:
pushal
pushfl 
cli
push $3
call do_irq
addl $4, %ESP
popfl
popal
iret 

irq4:
pushal
pushfl
cli
push $4
call do_irq
addl $4, %ESP
popfl
popal
iret 
  

irq5:
pushal
pushfl
cli
push $5
call do_irq
addl $4, %ESP
popfl
popal
iret 

irq6:
pushal
pushfl
cli
push $6
call do_irq
addl $4, %ESP
popfl
popal
iret 

irq7:
pushal
pushfl
cli
push $7
call do_irq
addl $4, %ESP
popfl
popal
iret 


irq8:
pushal
pushfl
cli
push $8
call do_irq
addl $4, %ESP
popfl
popal
iret 
  

irq9:
pushal
pushfl
cli
push $9
call do_irq
addl $4, %ESP
popfl
popal
iret 

irq10:
pushal
pushfl
cli
push $10
call do_irq
addl $4, %ESP
popfl
popal
iret 

irq11:
pushal
pushfl
cli
push $11
call do_irq
addl $4, %ESP
popfl
popal
iret 


irq12:
pushal
pushfl
cli
push $12
call do_irq
addl $4, %ESP
popfl
popal
iret 
  

irq13:
pushal
pushfl
cli
push $13
call do_irq
addl $4, %ESP
popfl
popal
iret 

irq14:
popl %eax
movl %eax, pagefault 
pushal
pushfl
cli
push $14
call do_irq
addl $4, %ESP
popfl
popal
iret 

# irq15:
# pushal
# pushfl
# push $15
# call do_irq
# addl $4, %ESP
# popfl
# popal
# iret 


irq16:
pushal
pushfl
cli
push $16
call do_irq
addl $4, %ESP
popfl
popal
iret 
  

irq17:
pushal
pushfl
cli
push $17
call do_irq
addl $4, %ESP
popfl
popal
iret 

irq18:
pushal
pushfl
cli
push $18
call do_irq
addl $4, %ESP
popfl
popal
iret 

irq19:
pushal
pushfl
cli
push $19
call do_irq
addl $4, %ESP
popfl
popal
iret 

irq20:
pushal
pushfl
cli
push $20
call do_irq
addl $4, %ESP
popfl
popal
iret 

irq32:
pushal
pushfl
cli
push $32
call do_irq
addl $4, %ESP
popfl
popal
iret 

irq33:
pushal
pushfl
cli
push $33
call do_irq
addl $4, %ESP
popfl
popal
iret 

irq40:
pushal
pushfl
cli
push $40
call do_irq
addl $4, %ESP
popfl
popal
iret 


# SYSCALLS
irq128:
pushal
# pushfl
sti
movl $0xFFFFFFFF,syscall_ret

cmpl $1, %eax
jl invalid
cmpl $10, %eax
jg invalid
# three arguments
pushl %edx
pushl %ecx
pushl %ebx
# decl %eax # start from 0 actually
call *systemcalls(,%eax,4)
addl $12, %ESP

movl %eax,syscall_ret

invalid:
# popfl
popal

movl syscall_ret, %eax

iret

