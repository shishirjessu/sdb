.global main 

.section .data 

hex_format: .asciz "%#x"

.section .text 

.macro trap 
    movq $62, %rax
    # Assuming the PID was put into r12
    movq %r12, %rdi
    # 5 is the TRAP ID for the kill syscall
    movq $5, %rsi
    syscall
.endm

# program entry point
main:
    # rbp contains start of stack frame. Save it 
    # by pushing onto stack so the function we call
    # can mess with it  
    push %rbp 

    # Now the new start of the stack pointer when we do the
    # call is the CURRENT stack ptr position
    movq %rsp, %rbp

    # Get PID. PID syscall is 39, and you syscall by moving
    # the syscall ID into rax, with subsequent args into 
    # rdi and rsi
    movq $39, %rax
    syscall 
    movq %rax, %r12

    trap

    leaq hex_format(%rip), %rdi
    movq $0, %rax 
    call printf@plt 
    movq $0, %rdi
    call fflush@plt
    trap 

    # pop off the initial value of rbp, and return with 0 
    # exit code
    popq %rbp 
    movq $0, %rax
    ret 

