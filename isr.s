.macro ISR_NOERRCODE num
.global isr\num
isr\num:
    cli                   # Désactiver les interruptions
    push $0               # Pousser un code d'erreur fictif (nécessaire pour l'alignement)
    push $\num            # Pousser le numéro de l'interruption
    jmp isr_common_stub   # Sauter vers la routine commune
.endm

ISR_NOERRCODE 0
ISR_NOERRCODE 32

isr_common_stub:
    pusha                 # Sauvegarder tous les registres (EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI)
    
    mov %ds, %ax          # Sauvegarder le segment de données
    push %eax
    
    mov $0x10, %ax        # Charger le sélecteur de segment noyau
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    
    call isr_handler      # APPEL DU CODE C
    
    pop %eax              # Restaurer le segment de données
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    
    popa                  # Restaurer tous les registres
    add $8, %esp          # Nettoyer le code d'erreur et le numéro d'ISR
    sti                   # Réactiver les interruptions
    iret
