;;
section .rodata
    line: db 10
section .data
    ourInputFile: dd 0
    ourOutputFile: dd 1
    my_Char: db 0
section .bss
    Length_: resd 1
section .text
global main
extern strlen
; help functions added before main
; they wouldn't be reached unless called

OpenInput:
    pushad
    mov eax, 0x5                   
    mov ebx, ecx                    
    inc ebx
    inc ebx                     
    xor ecx, ecx                    
    int 0x80                        
    mov dword[ourInputFile], eax         
    popad
    jmp continuedInLoop  

OpenOutput:
    pushad
    mov eax, 0x5                    
    mov ebx, ecx                    
    inc ebx
    inc ebx
    mov ecx, 101o                   
    mov edx, 777o                   
    int 0x80                       
    mov dword[ ourOutputFile], eax        
    popad
    jmp continuedInLoop     

main:
PrepVars:
    mov edi, dword[esp+8]      
    mov esi, dword[esp+4]       
    and eax, 0                 

PrintArgs:
    StartingLoop:
        mov ecx, dword[edi + eax * 4]  
    getLenOfArgv:
        pushad                        
        push ecx                       
        call strlen                     
        pop ecx                       
        mov dword[Length_], eax      
    PrintingArgv:
        mov eax, 0x4                     
        xor ebx, ebx
        inc ebx                     
        mov ecx, ecx                 
        mov edx, dword[Length_]            
        int 0x80                      
    
        mov eax, 0x4                   
        mov ebx, 1                      
        mov ecx, line             
        mov edx, 1                     
        int 0x80                       
        popad   

    oneCheckForArgv:
        cmp word[ecx], "-i"            
        je OpenInput   
    secondCheckForArgv:              
        cmp word[ecx], "-o"          
        je OpenOutput

    continuedInLoop:
        inc eax     
        cmp eax, esi           
        jne StartingLoop  
        
Encoder:
    gettingChar:   
        xor eax, eax
        or eax, 0x3               
        mov ebx, dword[ourInputFile]    
        mov ecx, my_Char           
        xor edx, edx
        inc edx
        int 0x80                   
    CheckingRead:   ;if(eof) then we should EXIT
        sub eax, 0                 
        jle EXIT                    
    comparingChar:   ;check range of my_Char
        mov al, 'A'
        cmp byte [my_Char], al      
        jl printingChar               
        mov bl, 'z'
        cmp byte [my_Char], bl      
        jg printingChar               
    encodingChar:    ;to increase my_Char by one
        inc byte[my_Char]        
    printingChar: 
        xor eax, eax
        mov eax, 0x4               
        mov ebx, dword[ourOutputFile]     
        mov ecx, my_Char             
        inc edx                  
        int 0x80                    
        jmp Encoder                

EXIT:
    mov eax, 0x1                    
    mov ebx, 0                     
    int 0x80