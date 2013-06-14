%INCLUDE "common.asm"

;=============================================================================
; Code
;=============================================================================

SECTION .text

%MACRO MT_LOGIC_8    3

   ASM_START_MMX

   movau             m0, [pbDestination + xax]
   movau             m1, [pbSource + xax]
   
   %1                m0, m6
   %2                m1, m7
   %3                m0, m1
   
   movau             [pbDestination + xax], m0
   
   ASM_STOP_MMX

%ENDMACRO

%MACRO MT_LOGIC_64   3
   prefetchrw        [pbDestination + xax + 384]
   prefetchr         [pbSource + xax + 384]

   %ASSIGN offset 0
   %REP 64 / (4 * mfull)
   
      movau          m0, [pbDestination + xax + offset + mfull * 0]
      movau          m1, [pbDestination + xax + offset + mfull * 1]
      movau          m4, [pbSource + xax + offset + mfull * 0]
      movau          m5, [pbSource + xax + offset + mfull * 1]
      
      %1             m0, m6
      %1             m1, m6
      %2             m4, m7
      %2             m5, m7
      
      movau          m2, [pbDestination + xax + offset + mfull * 2]
      movau          m3, [pbDestination + xax + offset + mfull * 3]
      
      %3             m0, m4
      %3             m1, m5

      movau          m4, [pbSource + xax + offset + mfull * 2]
      movau          m5, [pbSource + xax + offset + mfull * 3]
      
      %1             m2, m6
      %1             m3, m6
      %2             m4, m7
      %2             m5, m7
      
      movau          [pbDestination + xax + offset + mfull * 0], m0
      movau          [pbDestination + xax + offset + mfull * 1], m1
      
      %3             m2, m4
      %3             m3, m5
      
      movau          [pbDestination + xax + offset + mfull * 2], m2
      movau          [pbDestination + xax + offset + mfull * 3], m3
   
      %ASSIGN offset offset + mfull * 4
   
   %ENDREP
%ENDMACRO

%MACRO MT_LOGIC_ENDLINE 0
   add                  pbDestination, nDestinationPitch
   add                  pbSource, nSourcePitch
%ENDMACRO

%MACRO MT_LOGIC_COMMON  4
   STACK                xax, xdx,, pbDestination, *, nDestinationPitch, pbSource, *, nSourcePitch, nWidth, *, nHeight, *, nThresholdDestination, nThresholdSource
   ASM_XMMX             %1
   
   movd                 m6, nThresholdDestination
   movd                 m7, nThresholdSource
   psplatbr             m6
   psplatbr             m7
   
   ASM_START_MMX

   movd                 m6, nThresholdDestination
   movd                 m7, nThresholdSource
   psplatbr             m6
   psplatbr             m7

   ASM_STOP_MMX
   
   MT_UNROLL_INPLACE_WIDTH  { MT_LOGIC_64 %{2:4} }, { MT_LOGIC_8 %{2:4} }, MT_LOGIC_ENDLINE
   
   RETURN
%ENDMACRO

%MACRO MT_LOGIC 2-*

   %DEFINE name %1
   %DEFINE instruction %{2:4}
   
   %REP %0 - 4
   
mt_mangle Logic, name %+ 8_ %+ %5
name %+ 8_ %+ %5:
      MT_LOGIC_COMMON %5, instruction
   
      %ROTATE 1   
      
   %ENDREP
   
%ENDMACRO

MT_LOGIC and      , ASM_NOP, ASM_NOP, pand  , mmx, isse, 3dnow, sse2, asse2
MT_LOGIC andn     , ASM_NOP, ASM_NOP, pandn , mmx, isse, 3dnow, sse2, asse2
MT_LOGIC or       , ASM_NOP, ASM_NOP, por   , mmx, isse, 3dnow, sse2, asse2
MT_LOGIC xor      , ASM_NOP, ASM_NOP, pxor  , mmx, isse, 3dnow, sse2, asse2

%MACRO MT_MULTI_LOGIC 3
MT_LOGIC %1   , %2, ASM_NOP, p%3ub, isse, 3dnow, sse2, asse2
MT_LOGIC %1sub, %2, psubusb, p%3ub, isse, 3dnow, sse2, asse2
MT_LOGIC %1add, %2, paddusb, p%3ub, isse, 3dnow, sse2, asse2
%ENDMACRO

%MACRO MT_MULTI_LOGIC_SQUARED 1
MT_MULTI_LOGIC    %1, ASM_NOP, %1
MT_MULTI_LOGIC sub%1, psubusb, %1
MT_MULTI_LOGIC add%1, paddusb, %1
%ENDMACRO

MT_MULTI_LOGIC_SQUARED min
MT_MULTI_LOGIC_SQUARED max

