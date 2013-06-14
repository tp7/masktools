%INCLUDE "common.asm"

%MACRO MT_SUPPORT_DIFF_8   1

   ASM_START_MMX

   movau                m0, [pbDestination + xax]
   movau                m1, [pbSource + xax]
   
   psubb                m0, m7
   psubb                m1, m7
   %1                   m0, m1
   paddb                m0, m7
   
   movau                [pbDestination + xax], m0
   
   ASM_STOP_MMX
   
%ENDMACRO

%MACRO MT_SUPPORT_DIFF_64  1
   prefetchrw           [pbDestination + xax + 128]
   prefetchr            [pbSource      + xax + 128]

   %ASSIGN offset 0
   %REP 64 / (mfull * 2)

      movau             m0, [pbDestination + xax + offset + 0 * mfull]
      movau             m4, [pbSource      + xax + offset + 0 * mfull]

      movau             m1, [pbDestination + xax + offset + 1 * mfull]
      movau             m5, [pbSource      + xax + offset + 1 * mfull]

      psubb             m0, m7
      psubb             m4, m7

      psubb             m1, m7
      psubb             m5, m7

      %1                m0, m4
      %1                m1, m5

      paddb             m0, m7
      paddb             m1, m7
      
      movau             [pbDestination + xax + offset + 0 * mfull], m0
      movau             [pbDestination + xax + offset + 1 * mfull], m1
      
      %ASSIGN offset offset + mfull * 2
      
   %ENDREP   
%ENDMACRO

%MACRO MT_SUPPORT_COMMON 3
%2 %+ 8_ %+ %1:
   STACK                xax, xdx,, pbDestination, *, nDestinationPitch, pbSource, *, nSourcePitch, nWidth, *, nHeight, *
   ASM_XMMX             %1
   
   mova                 m7, [ASM_BYTE_0x80]
   movq                 mm7, [ASM_BYTE_0x80]
   
   MT_UNROLL_INPLACE_WIDTH  MT_ %+ %3 %+ _64, MT_ %+ %3 %+ _8, MT_ %+ %3 %+ _ENDLINE
      
   RETURN
%ENDMACRO