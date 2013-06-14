%INCLUDE "common.asm"

;=============================================================================
; Code
;=============================================================================

SECTION .text

mt_mangle Invert, invert8_mmx
mt_mangle Invert, invert8_isse
mt_mangle Invert, invert8_3dnow
mt_mangle Invert, invert8_sse2
mt_mangle Invert, invert8_asse2
   
%MACRO MT_INVERT_8 0
   ASM_START_MMX
   movau                m0, [pbDestination + xax]
   pxor                 m0, m7
   movau                [pbDestination + xax], m0
   ASM_STOP_MMX
%ENDMACRO

%MACRO MT_INVERT_64 0
   
   %ASSIGN offset 0   
   %REP 64 / (mfull * 4)

      movau             m0, [pbDestination + xax + offset + 0 * mfull]
      movau             m1, [pbDestination + xax + offset + 1 * mfull]
      movau             m2, [pbDestination + xax + offset + 2 * mfull]
      movau             m3, [pbDestination + xax + offset + 3 * mfull]
      pxor              m0, m7
      pxor              m1, m7
      pxor              m2, m7
      pxor              m3, m7
      movau             [pbDestination + xax + offset + 0 * mfull], m0
      movau             [pbDestination + xax + offset + 1 * mfull], m1
      movau             [pbDestination + xax + offset + 2 * mfull], m2
      movau             [pbDestination + xax + offset + 3 * mfull], m3
   
      %ASSIGN offset offset + 4 * mfull
      
   %ENDREP   

%ENDMACRO

%MACRO MT_INVERT_ENDLINE 0
   add                  pbDestination, nDestinationPitch
%ENDMACRO

%MACRO MT_INVERT_COMMON 1
invert8_ %+ %1:
   STACK                xax, xdx,, pbDestination, *, nDestinationPitch, *, nWidth, *, nHeight, *
   ASM_XMMX             %1
   
   pcmpeqb              m7, m7
   pcmpeqb              mm7, mm7
   
   MT_UNROLL_INPLACE_WIDTH MT_INVERT_64, MT_INVERT_8, MT_INVERT_ENDLINE
   
   RETURN
%ENDMACRO

MT_INVERT_COMMON     mmx
MT_INVERT_COMMON     isse
MT_INVERT_COMMON     3dnow
MT_INVERT_COMMON     sse2
MT_INVERT_COMMON     asse2
