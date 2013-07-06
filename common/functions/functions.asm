%INCLUDE "common.asm"

;=============================================================================
; Code
;=============================================================================

SECTION .text

   ASM_GLOBAL cpuid_asm
   ASM_GLOBAL cpu_test_asm
   

cpuid_asm:
   STACK xax, xbx, xcx, xdx, p,, nOp, pnEax, pnEbx, pnEcx, pnEdx
   
   mov         xax, nOp
   cpuid       
   mov         p, pnEax
   mov         [p], xaxd
   mov         p, pnEbx
   mov         [p], xbxd
   mov         p, pnEcx
   mov         [p], xcxd
   mov         p, pnEdx
   mov         [p], xdxd
   
   RETURN

%IFIDNI ASM_CHIP, x64

cpu_test_asm:
   mov         rax, 1
   ret
   
%ELSE   

cpu_test_asm:
    pushfd
    push    ebx
    push    ebp
    push    esi
    push    edi

    pushfd
    pop     eax
    mov     ebx, eax
    xor     eax, 0x200000
    push    eax
    popfd
    pushfd
    pop     eax
    xor     eax, ebx
    
    pop     edi
    pop     esi
    pop     ebp
    pop     ebx
    popfd
    ret
   
%ENDIF   
