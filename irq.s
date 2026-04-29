.global irq0
irq0:
    cli
    push $0        # Code d'erreur fictif
    push $32       # Numéro d'interruption (32 = IRQ 0)
    jmp irq_common_stub

.global irq_common_stub
irq_common_stub:
    pusha          # Sauvegarde des registres
    mov %ds, %ax
    push %eax
    
    mov $0x10, %ax # Charge le segment de données noyau
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    
    call isr_handler # On appelle un handler spécifique aux IRQ
    
    pop %eax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    popa
    add $8, %esp
    sti
    iret
