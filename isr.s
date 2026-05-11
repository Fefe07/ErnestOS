.macro ISR_NOERRCODE num
.global isr\num
isr\num:
    cli
    push $0
    push $\num
    jmp isr_common_stub
.endm

ISR_NOERRCODE 0
ISR_NOERRCODE 32
ISR_NOERRCODE 33
ISR_NOERRCODE 129

isr_common_stub:
    pusha
    mov %ds, %ax
    push %eax
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es

    push %esp
    call isr_handler
    mov %eax, %esp

.global isr_restore_context
isr_restore_context:
    pop %eax
    mov %ax, %ds
    mov %ax, %es
    popa
    add $8, %esp
    iret
