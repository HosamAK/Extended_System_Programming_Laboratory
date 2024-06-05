section .bss
    buffer: resb 600
    struct: resd 1

section .data
    x_struct: db 5
    x_num: db 0xaa, 1,2,0x44,0x4f
    help1: db 10, 20, 30, 40, 50
    y_struct: db 6
    y_num: db 0xaa, 1,2,3,0x44,0x4f
    testing: db 10, 20, 30 ,40, 50, 60, 70, 80, 90, 100
    hexaf: db "%02hhx", 0
    MASK: dw 0x002D
    help2: db 10, 20, 30, 40, 50
    new_line: db 10,0
    STATE: dw 0xACE1 ;status
    help3: db 10, 20, 30, 40, 50
    sml: db 0
    help4: db 10, 20, 30, 40, 50
    big: db 0

global main
extern stdin
extern free
extern strlen
extern printf
extern malloc
extern fgets

func1:
    xor eax, eax
    mov ebx, 0
    ret

func2:
    mov ecx, 5
    dec ecx
    ret

func3:
    mov ecx, 5
    mov edi, help1
    mov al, 10
    iamfilling333:
        mov [edi], al
        inc edi
        loop iamfilling333
    ret

func4:
    xor edx, edx
    inc edx
    dec edx
    inc edx
    inc edx
    ret

func5:
    mov ecx, 5
    mov edi, help2
    mov al, 5
    fillfor5:
        mov [edi], al
        inc edi
        loop fillfor5
    ret

func6:
    mov ecx, 3
    mov edi, help4
    mov al, 10
    letsfill6:
        mov [edi], al
        inc edi
        loop letsfill6
    mov ecx, 5
    mov edi, help4
    looping_multi:
        movzx eax, byte [edi]
        add al, al
        mov [edi], al
        inc edi
        loop looping_multi
    ret

func7:
    call test
    call update_ui
    mov ecx, 5
    mov edi, help3
    mov al, 33
    herewegofilling7:
        mov [edi], al
        inc edi
        loop herewegofilling7
    mov ecx, 5
    mov edi, help3
    looping_divide:
        movzx eax, byte [edi]
        xor edx, edx
        mov ebx, 3
        div bl
        mov [edi], al
        inc edi
        loop looping_divide
    ret

main:
    call func1
    call func2
    call func3
    call func4
    call func5
    call func6
    call func7
    call test
    mov eax, [esp+4]             ;argc
    mov ebx, [esp+8]             ;argv

    cmp eax, 1                   ;if argc 1 no arguments given
    je normal                    ;add saved structs and print
    mov eax, [ebx + 4]           ;check which flag given
    cmp word[eax], "-R"          ;case 1 : -R
    je case1
    cmp word[eax], "-I"          ;case 2 : -I
    je case2

    ret                          ;finished

case1:
    call PRmulti                 ;generate random number
    push eax
    call PRmulti                 ;generate random number
    push eax
    call add_multi               ;sum the generated random numbers
    push eax
    call print_multi             ;print sum
    jmp freepush                 ;clean stack

case2:
    call getmulti                ;recieve number from input (stdin)
    push eax
    call getmulti                ;recieve number from input (stdin)
    push eax
    call add_multi               ;sum the two numbers
    push eax
    call print_multi             ;print sum
    call hello_vug
    jmp freepush                 ;clean stack

normal:
    push y_struct                ;second argument
    push x_struct                ;first argument
    call add_multi               ;sum two structs
    push eax
    call print_multi             ;print sum
    call free                    ;clean allocation
    add esp, 12                  ;clean stack
    ret

print_multi:
    push ebp                     ;create new activation frame
    mov ebp, esp
    pushad
    mov esi, [ebp+8]
    xor ebx, ebx                 ;clean ebx
    mov bl, byte[esi]            ;struct size : counter

print_all:
    pushad
    mov al, byte[esi + ebx]      ;end of num[i]
    push eax                     ;second argument
    push hexaf                   ;first argument
    call printf                  ;printf("%02hhx", (*struct).num[i])
    add esp, 8
    popad
    sub ebx, 2
    add ebx, 1
    jnz print_all                ;if ebx not zero jump print_all

    push new_line                ;"\n"
    call printf                  ;printf("\n");
    add esp, 4

    popad
    pop ebp
    ret

getmulti:
    push ebp
    mov ebp, esp
    pushad
    push dword[stdin]            ;third argument: stdin address to read from stdin
    push 600                     ;second argument: size of input to read
    push buffer                  ;first argument: saved input to buffer
    call fgets                   ;fgets(buffer, 600, stdin)
    add esp, 12
    push buffer
    call strlen                  ;the lenth of buffer
    add esp, 4
    mov edi, eax                 ;save to edi buffer's length
    sub edi, 3                   ;calc the right length excluding '\n'
    add edi, 1
    shr eax, 1                   ;calc the size of required arr
    add eax, 2                   ;the size of required arr + its size (1)
    sub eax, 1
    push eax                     ;arguemnt for malloc
    call malloc                  ;the required allocated size
    mov dword[struct], eax       ;using later to return it as a result
    mov esi, eax                 ;save eax value
    pop eax
    sub eax, 2
    add eax, 1
    mov byte[esi], al
    mov ecx, 1                   ;counter of recieved input skipping its size (0 index)

stdn:
    cmp edi, 0                  
    jl finished_input             ;jump if finished scanning
    xor ebx, ebx                 ;clean ebx
    mov bh, byte[buffer + edi]   ;first char
    sub edi, 2
    add edi, 1
    call parse_digit             ;parser from char to hexa
    mov bl, bh
    mov bh, 0                    ;clean bh
    cmp edi, 0
    jl Bytecreate
    mov bh, byte[buffer + edi]   ;second char
    sub edi, 2
    add edi, 1
    call parse_digit             ;parse char to hexa

Bytecreate:
    shl bh, 4                    ;trun 2 chars into 1 byte of hexa
    or bl, bh                    ;after merge the two chars

    mov byte[esi + ecx], bl
    add ecx, 2
    sub ecx, 1
    jmp stdn                     ;loop

parse_digit:
    cmp bh, '9'
    jle parse_number             ;number
    sub bh, 'a'                  ;letter
    add bh, 0xa                 
    ret

parse_number:
    sub bh, '0'                  ;ascii to real value
    ret

finished_input:
    popad
    mov eax, dword[struct]       ;eax contains the required value from the function (struct)
    pop ebp
    ret

minimum_maximum:
    call deal_with
    movzx ecx, byte[eax]         ;size of the first argument
    movzx edx, byte[ebx]         ;size of the second argument
    cmp ecx, edx                 ;if first is bigger than the second
    jae noSwap                   ;register have the right values
    xchg eax, ebx                ;swap registers values
    noSwap:            
    ret

hello_vug:
    jmp deal_with
    push ebp
    mov ebp, esp
    pop ebp
    ret

add_multi:
    push ebp
    mov ebp, esp
    pushad

    mov eax, [ebp+8]         ;get first argument (first struct)
    mov ebx, [ebp+12]        ;get second argument (second struct)
    call minimum_maximum     ;eax has the bigger struct and ebx has the second
    
    ;print both structs and clean stack
    push ebx
    call print_multi
    add esp, 4
    push eax
    call print_multi
    add esp, 4

    ;esi has the bigger struct and edi has the second struct
    mov esi, eax
    mov edi, ebx

    movzx eax, byte[edi]
    mov byte[sml], al
    movzx eax, byte[esi]
    mov byte[big], al
    add eax, 2               ;addition 2 to eax (size) one for the size it self and one for the carry (just in case)
    push eax
    call malloc              ;allocate size of eax for struct in heap
    mov dword[struct], eax   ;the address of the allocated memory is saved
    pop ecx                  ;update ecx to contain the size of the new allocated struct
    sub ecx, 2               ;remove the the one byte we added before the size it self
    add ecx, 1
    mov byte[eax], cl        ;the first byte contains num[] size
    xor ecx, ecx             ;clean ecx - ecx is the curry for addition
    mov edx, 0               ;clean edx - edx is the counter for the loop

    ;esi, edi and eax point to the first byte in the array of num
    add esi, 2
    sub esi, 1
    add edi, 2
    sub edi, 1
    add eax, 2
    sub eax, 1

; loop for add
helper_loop:
    call start_count
    movzx ebx, byte[esi]
    add ebx, ecx
    movzx ecx, byte[edi]
    add ebx, ecx
    mov cl, bh
    mov byte[eax], bl
    add edx, 2
    sub edx, 1
    add esi, 2
    sub esi, 1
    add edi, 2
    sub edi, 1
    add eax, 2
    sub eax, 1
    cmp dl, byte[sml]
    jne helper_loop

    cmp dl, byte[big]
    je skipAdd

another_helper_loop:
    movzx ebx, byte[esi]
    add ebx, ecx
    mov cl, bh
    mov byte[eax], bl
    add edx, 2
    sub edx, 1
    add esi, 2
    sub esi, 1
    add eax, 2
    sub eax, 1
    cmp dl, byte[big]
    jne another_helper_loop
    skipAdd:
    mov byte[eax], cl
    popad
    pop ebp
    mov eax, dword[struct]
    ret

PRmulti:
    push ebp
    mov ebp, esp
    pushad

get_number:
    call generate_rnd
    cmp al, 0
    je get_number

    movzx ebx, al
    add ebx, 3
    sub ebx, 2
    push ebx

    call malloc
    call update_ui
    mov dword[struct], eax
    pop ebx
    sub ebx, 2
    add ebx, 1
    mov byte[eax], bl
    xor ecx, ecx
    mov esi, eax
    xor edx, edx
    looper:
        call generate_rnd
        mov byte[esi + edx + 1], al
        add edx, 2
        sub edx, 1
        sub ebx, 2
        add ebx, 1
        jnz looper
    popad
    pop ebp
    mov eax, [struct]
    ret

deal_with:
    jmp start_count
    push ebp
    mov ebp, esp
    pop ebp
    ret

start_count:
    push ebp
    mov ebp, esp
    pop ebp
    ret

test:
    mov ecx, 10
    mov edi, testing
    mov al, 55
    looping_test:
        mov [edi], al
        inc al
        inc edi
        loop looping_test
    mov ecx, 10
    mov edi, testing
    jmp hello_vug
    looping_mul:
        movzx eax, byte [edi]
        add al, al
        mov [edi], al
        inc edi
        loop looping_mul
    mov ecx, 10
    mov edi, testing
    jmp update_ui
    final_stage:
        movzx eax, byte [edi]
        xor edx, edx
        mov ebx, 7
        div bl
        mov [edi], al
        inc edi
        loop final_stage
    ret

update_ui:
    jmp hello_vug
    push ebp
    mov ebp, esp
    pop ebp
    ret

freepush:                       ;clean stack
    call free
    add esp, 4
    call free
    add esp, 4
    call free
    add esp, 4
    ret

generate_rnd:
    push ebp
    mov ebp, esp
    pushad
    ; loop random gen
    movzx eax, word[STATE]
    and ax, [MASK]
    mov bx, 0
    making:
        movzx ecx, ax
        and cx, 1
        xor bx, cx
        shr ax, 1
        jnz making
    
    movzx eax, word[STATE]
    shr ax, 1
    shl bx, 15
    or ax, bx
    mov word[STATE], ax
    
    popad
    pop ebp
    movzx eax, word[STATE]
    ret