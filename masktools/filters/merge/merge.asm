%INCLUDE "common.asm"

;=============================================================================
; Code
;=============================================================================

SECTION .text

mt_mangle Merge, merge8_mmx
mt_mangle Merge, merge8_isse
mt_mangle Merge, merge8_3dnow
mt_mangle Merge, merge8_sse2
mt_mangle Merge, merge8_asse2
mt_mangle Merge, merge_luma_4208_mmx
mt_mangle Merge, merge_luma_4208_isse
mt_mangle Merge, merge_luma_4208_3dnow
mt_mangle Merge, merge_luma_4208_sse2

%MACRO MT_LOAD_MASK_420 1

   movau                m0, [pbMaskUp   + 2 * xax + 2 * %1 + 0 * mfull]
   movau                m1, [pbMaskUp   + 2 * xax + 2 * %1 + 1 * mfull]

   %IFIDNI movau, mova

      pavgb             m0, [pbMaskDown + 2 * xax + 2 * %1 + 0 * mfull]
      pavgb             m1, [pbMaskDown + 2 * xax + 2 * %1 + 1 * mfull]
      
   %ELSE

      movu              m2, [pbMaskDown + 2 * xax + 2 * %1 + 0 * mfull]
      movu              m3, [pbMaskDown + 2 * xax + 2 * %1 + 1 * mfull]
      pavgb             m0, m2
      pavgb             m1, m3
      
   %ENDIF
   
   mova                 m2, m0
   mova                 m3, m1
   pshift               m2, -1
   pshift               m3, -1
   pavgb                m0, m2
   pavgb                m1, m3
   pand                 m0, [ASM_WORD_0x00FF]
   pand                 m1, [ASM_WORD_0x00FF]
      
%ENDMACRO    

%MACRO MT_LOAD_MASK     1
   movh                 m0, [pbMask + xax + %1 + 0 * mhalf]
   movh                 m1, [pbMask + xax + %1 + 1 * mhalf]
   punpcklbw            m0, m7
   punpcklbw            m1, m7
%ENDMACRO

%MACRO MT_MERGE_CORE    2

   %1                   %2
   
   movh                 m2, [pbDestination + xax + %2 + 0 * mhalf]
   movh                 m4, [pbSource      + xax + %2 + 0 * mhalf]
   mova                 m6, [ASM_WORD_0x0100]
   punpcklbw            m2, m7
   punpcklbw            m4, m7
   psubw                m6, m0
   movh                 m3, [pbDestination + xax + %2 + 1 * mhalf]
   movh                 m5, [pbSource      + xax + %2 + 1 * mhalf]
   pmullw               m2, m6
   pmullw               m4, m0
   mova                 m6, [ASM_WORD_0x0100]
   punpcklbw            m3, m7
   punpcklbw            m5, m7
   psubw                m6, m1
   paddw                m2, [ASM_WORD_0x0080]
   pmullw               m3, m6
   pmullw               m5, m1
   paddw                m3, [ASM_WORD_0x0080]
   paddw                m2, m4
   paddw                m3, m5
   psrlw                m2, 8
   psrlw                m3, 8
   
   packuswb             m2, m3
   
   movau                [pbDestination + xax + %2], m2
   
%ENDMACRO

%MACRO MT_MERGE_8       0
   ASM_START_MMX
   MT_MERGE_CORE        MT_LOAD_MASK, 0
   ASM_STOP_MMX
%ENDMACRO

%MACRO MT_MERGE_420_8   0
   ASM_START_MMX
   MT_MERGE_CORE        MT_LOAD_MASK_420, 0
   ASM_STOP_MMX
%ENDMACRO

%MACRO MT_MERGE_64      0

   prefetchrw           [pbDestination + xax + 64]
   prefetchr            [pbSource      + xax + 64]
   prefetchr            [pbMask        + xax + 64]
   
   %ASSIGN offset 0
   %REP 64 / mfull
   
      MT_MERGE_CORE     MT_LOAD_MASK, offset
      
      %ASSIGN offset    offset + mfull
      
   %ENDREP
   
%ENDMACRO

%MACRO MT_MERGE_420_64  0

   prefetchrw           [pbDestination + xax + 64]
   prefetchr            [pbSource      + xax + 64]
   
   %ASSIGN offset 0
   %REP 64 / mfull
   
      MT_MERGE_CORE     MT_LOAD_MASK_420, offset
      
      %ASSIGN offset    offset + mfull
      
   %ENDREP
   
%ENDMACRO

%MACRO MT_MERGE_ENDLINE 0
   add                  pbDestination, nDestinationPitch
   add                  pbSource, nSourcePitch
   add                  pbMask, nMaskPitch
%ENDMACRO

%MACRO MT_MERGE_420_ENDLINE 0
   add                  pbDestination, nDestinationPitch
   add                  pbSource, nSourcePitch
   add                  pbMaskDown, nMaskPitch
   mov                  pbMaskUp, pbMaskDown
   add                  pbMaskDown, nMaskPitch
%ENDMACRO

%MACRO MT_MERGE_COMMON  2
%2 %+ 8_ %+ %1:
   STACK                xax, xdx,, pbDestination, *, nDestinationPitch, pbSource, *, nSourcePitch, pbMask, *, nMaskPitch, nWidth, *, nHeight, *
   ASM_XMMX             %1

   pxor                 m7, m7
   pxor                 mm7, mm7
   
   MT_UNROLL_INPLACE_WIDTH  MT_MERGE_64, MT_MERGE_8, MT_MERGE_ENDLINE
      
   RETURN
%ENDMACRO

%MACRO MT_MERGE_420_COMMON  2
%2 %+ 8_ %+ %1:
   STACK                xax, xdx, pbMaskDown,, pbDestination, *, nDestinationPitch, pbSource, *, nSourcePitch, pbMaskUp, *, nMaskPitch, nWidth, nHeight, *
   ASM_XMMX             %1

   pxor                 m7, m7
   pxor                 mm7, mm7
   mov                  pbMaskDown, pbMaskUp
   add                  pbMaskDown, nMaskPitch
   
   MT_UNROLL_INPLACE_WIDTH  MT_MERGE_420_64, MT_MERGE_420_8, MT_MERGE_420_ENDLINE
      
   RETURN
%ENDMACRO

MT_MERGE_COMMON mmx, merge
MT_MERGE_COMMON isse, merge
MT_MERGE_COMMON 3dnow, merge
MT_MERGE_COMMON sse2, merge
MT_MERGE_COMMON asse2, merge

MT_MERGE_420_COMMON mmx, merge_luma_420
MT_MERGE_420_COMMON isse, merge_luma_420
MT_MERGE_420_COMMON 3dnow, merge_luma_420
MT_MERGE_420_COMMON sse2, merge_luma_420

