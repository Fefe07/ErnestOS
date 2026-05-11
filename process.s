.global switch_to
switch_to:
    # Sauvegarde des registres de la tâche actuelle
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %ebx

    movl 20(%esp), %eax
    movl %esp, (%eax)

    movl 24(%esp), %esp

    popl %ebx
    popl %esi
    popl %edi
    popl %ebp

    ret
