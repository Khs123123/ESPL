section .data
infile  dd 0
outfile dd 1
newline db 10
errmsg db "Error opening file", 10
errmsg_len equ $ - errmsg
startmsg db "Starting encoding...", 10
startmsg_len equ $ - startmsg

section .bss
buf resb 1
filename_buf resb 128

section .text
global _start
extern strlen

_start:
    ; Set default descriptors
    mov dword [infile], 0
    mov dword [outfile], 1

    ; -------- Task 1.A: print argv[i] line by line to stderr --------
    mov ecx, [esp]         ; argc
    lea esi, [esp+4]       ; argv

.arg_print_loop:
    cmp ecx, 0
    je .parse_args
    lodsd                  ; load next argv[i] into eax
    push ecx

    push eax               ; push argument address to call strlen
    call strlen
    add esp, 4
    mov edx, eax           ; strlen(argv[i])
    mov ecx, [esi - 4]     ; argv[i] pointer
    mov ebx, 2             ; stderr
    mov eax, 4             ; sys_write
    int 0x80

    ; write newline
    mov eax, 4
    mov ebx, 2
    mov ecx, newline
    mov edx, 1
    int 0x80

    pop ecx
    dec ecx
    jmp .arg_print_loop

    ; -------- Task 1.C: Parse -i and -o --------
.parse_args:
    mov ecx, [esp]
    lea esi, [esp+4]

.arg_loop:
    cmp ecx, 0
    je .start_encode
    lodsd
    push ecx

    ; -iinput.txt
    cmp byte [eax], '-'
    jne .next_arg
    cmp byte [eax+1], 'i'
    jne .check_o
    add eax, 2
    mov ebx, eax
    mov eax, 5         ; sys_open
    mov ecx, 0         ; O_RDONLY
    int 0x80
    cmp eax, 0
    jl .fail
    mov [infile], eax
    jmp .next_arg

.check_o:
    cmp byte [eax+1], 'o'
    jne .next_arg
    add eax, 2

    ; copy filename into filename_buf
    mov edi, filename_buf
.copy_filename:
    mov bl, [eax]
    mov [edi], bl
    cmp bl, 0
    je .open_output
    inc eax
    inc edi
    jmp .copy_filename

.open_output:
    mov ebx, filename_buf
    mov eax, 5
    mov ecx, 577       ; O_WRONLY | O_CREAT | O_TRUNC
    mov edx, 0644
    int 0x80
    cmp eax, 0
    jl .fail
    mov [outfile], eax

.next_arg:
    pop ecx
    dec ecx
    jmp .arg_loop

.fail:
    mov eax, 4
    mov ebx, 2
    mov ecx, errmsg
    mov edx, errmsg_len
    int 0x80
    jmp .exit

    ; -------- Task 1.B: Start encoding --------
.start_encode:
    mov eax, 4
    mov ebx, 2
    mov ecx, startmsg
    mov edx, startmsg_len
    int 0x80

.read_loop:
    mov eax, 3
    mov ebx, [infile]
    mov ecx, buf
    mov edx, 1
    int 0x80
    cmp eax, 0
    jle .exit

    movzx eax, byte [buf]
    cmp al, 'A'
    jl .write
    cmp al, 'Z'
    jg .write
    sub byte [buf], 1

.write:
    mov eax, 4
    mov ebx, [outfile]
    mov ecx, buf
    mov edx, 1
    int 0x80

    ; Debug to stderr
    mov eax, 4
    mov ebx, 2
    mov ecx, buf
    mov edx, 1
    int 0x80

    jmp .read_loop

.exit:
    mov eax, 1
    xor ebx, ebx
    int 0x80
