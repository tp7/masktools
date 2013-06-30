%INCLUDE "common.asm"

;=============================================================================
; Code
;=============================================================================

SECTION .text

   ASM_GLOBAL start_asm
   ASM_GLOBAL stop_asm
   ASM_GLOBAL cpuid_asm
   ASM_GLOBAL cpu_test_asm
   
%MACRO MT_MEMSET_8      0
   ASM_START_MMX
   movau                [pbDestination + xax], m0
   ASM_STOP_MMX
%ENDMACRO
   
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

start_asm:
   STACK pbBytes, *
   
   movdqu [pbBytes + 0 * 16], xmm6
   movdqu [pbBytes + 1 * 16], xmm7
   movdqu [pbBytes + 2 * 16], xmm8
   movdqu [pbBytes + 3 * 16], xmm9
   movdqu [pbBytes + 4 * 16], xmm10
   movdqu [pbBytes + 5 * 16], xmm11
   movdqu [pbBytes + 6 * 16], xmm12
   movdqu [pbBytes + 7 * 16], xmm13
   movdqu [pbBytes + 8 * 16], xmm14
   movdqu [pbBytes + 9 * 16], xmm15
   
   pxor xmm6, xmm6
   pxor xmm7, xmm7
   pxor xmm8, xmm8
   pxor xmm9, xmm9
   pxor xmm10, xmm10
   pxor xmm11, xmm11
   pxor xmm12, xmm12
   pxor xmm13, xmm13
   pxor xmm14, xmm14
   pxor xmm15, xmm15
  
   RETURN

stop_asm:
   STACK pbBytes, *

   movdqu  xmm6 , [pbBytes + 0 * 16]
   movdqu  xmm7 , [pbBytes + 1 * 16]
   movdqu  xmm8 , [pbBytes + 2 * 16]
   movdqu  xmm9 , [pbBytes + 3 * 16]
   movdqu  xmm10, [pbBytes + 4 * 16]
   movdqu  xmm11, [pbBytes + 5 * 16]
   movdqu  xmm12, [pbBytes + 6 * 16]
   movdqu  xmm13, [pbBytes + 7 * 16]
   movdqu  xmm14, [pbBytes + 8 * 16]
   movdqu  xmm15, [pbBytes + 9 * 16]

   RETURN

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

start_asm:
   STACK
   emms
   RETURN

stop_asm:
   STACK
   emms
   RETURN
   
%ENDIF   
