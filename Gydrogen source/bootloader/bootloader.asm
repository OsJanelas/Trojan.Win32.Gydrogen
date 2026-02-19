[bits 16]
[org 0x7c00]

start:
    ; Modo VGA 13h (320x200, 256 cores)
    mov ax, 0x0013
    int 0x10

    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:init_32bit

gdt_start:
    dq 0x0
gdt_code:
    dw 0xffff, 0x0000, 0x9a00, 0x00cf
gdt_data:
    dw 0xffff, 0x0000, 0x9200, 0x00cf
gdt_end:
gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

[bits 32]
init_32bit:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    
    xor ebp, ebp            ; Nosso Timer (EBP)

main_loop:
    inc ebp
    mov edi, 0xA0000        ; Aponta para a memória de vídeo
    xor ecx, ecx            ; Contador de pixels

render_pixel:
    mov eax, ecx
    xor edx, edx
    mov ebx, 320
    div ebx                 ; EAX = Y, EDX = X

    sub edx, 160            ; Centralizar X
    sub eax, 100            ; Centralizar Y

    ; Movimentação baseada no timer
    add edx, ebp
    sub eax, ebp

    ; --- VALOR ABSOLUTO ---
    cmp edx, 0
    jge .abs_x_ok
    neg edx
.abs_x_ok:
    cmp eax, 0
    jge .abs_y_ok
    neg eax
.abs_y_ok:

    ; --- VERIFICAÇÃO DO CUBO ---
    cmp edx, 50
    jg .draw_bg
    cmp eax, 50
    jg .draw_bg

    ; --- XOR FRACTAL RGB ---
    ; Em vez de bpl, movemos o timer para ebx e usamos bl
    mov ebx, ebp
    
    mov al, dl              ; AL = parte de X
    xor al, ah              ; XOR com parte de Y (ah veio do div lá atrás)
    add al, bl              ; bl é o byte baixo do timer (EBP)
    
    mov [edi], al
    jmp .next

.draw_bg:
    mov byte [edi], 0       ; Fundo Preto

.next:
    inc edi
    inc ecx
    cmp ecx, 64000
    jne render_pixel

    ; V-Sync para evitar que a tela pisque
    mov dx, 0x3DA
.wait:
    in al, dx
    test al, 8
    jz .wait

    jmp main_loop

times 510-($-$$) db 0
dw 0xaa55