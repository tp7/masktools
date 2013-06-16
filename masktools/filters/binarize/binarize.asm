%INCLUDE "common.asm"

;=============================================================================
; Code
;=============================================================================

SECTION .text

%MACRO MT_BINARIZE_8 4

   ASM_START_MMX
   
   movau                m0, [pbDestination + xax]
   
   %1                   m1, m0
   %2                   m0, m6
   %3                   m0, m7
   %4                   m0, m1, m5
   
   movau                [pbDestination + xax], m0
   
   ASM_STOP_MMX
   
%ENDMACRO

%MACRO MT_BINARIZE_64 4

   prefetchrw           [pbDestination + xax + 320]

   %ASSIGN offset 0
   %REP 64 / (mfull * 2)

      movau                m0, [pbDestination + xax + offset + 0 * mfull]
      movau                m1, [pbDestination + xax + offset + 1 * mfull]
      %1                   m3, m0
      %1                   m4, m1
      %2                   m0, m6
      %2                   m1, m6
      %3                   m0, m7
      %3                   m1, m7
      %4                   m0, m3, m5
      %4                   m1, m4, m5
      movau                [pbDestination + xax + offset + 0 * mfull], m0
      movau                [pbDestination + xax + offset + 1 * mfull], m1
      
      %ASSIGN offset offset + 2 * mfull
      
   %ENDREP   

%ENDMACRO      
   
%MACRO MT_AND_1_2 3
   pand %1, %2   
%ENDMACRO

%MACRO MT_AND_1_3 3
   pand %1, %3   
%ENDMACRO

%MACRO MT_OR_1_2 3
   por %1, %2   
%ENDMACRO

%MACRO MT_OR_1_3 3
   por %1, %3   
%ENDMACRO

%MACRO MT_BINARIZE_ENDLINE 0
   add                  pbDestination, nDestinationPitch
%ENDMACRO

%MACRO MT_BINARIZE_COMMON 6
mt_mangle Binarize, %2 %+ 8_ %+ %1
%2 %+ 8_ %+ %1 :
   STACK                xax, xdx,, pbDestination, *, nDestinationPitch, *, nThreshold, nWidth, *, nHeight, *
   ASM_XMMX             %1

   mov                  xax, nThreshold
   movd                 m5, xaxd
   movd                 mm5, xaxd
   psplatbr             m5
   
   %IFIDNI %5, pcmpgtb
   
      add               xax, 128
      movd              m7, xaxd
      psplatbr          m7
      mova              m6, [ASM_BYTE_0x80]

      ASM_START_MMX
      movd              m7, xaxd
      psplatbr          m7
      psplatbr          m5
      mova              m6, [ASM_BYTE_0x80]
      ASM_STOP_MMX
      
   %ELSE
   
      pxor              m7, m7
      movd              m6, xaxd
      psplatbr          m6
   
      ASM_START_MMX
      pxor              m7, m7
      movd              m6, xaxd
      psplatbr          m5
      psplatbr          m6
      ASM_STOP_MMX
   
   %ENDIF
         
   MT_UNROLL_INPLACE_WIDTH { MT_BINARIZE_64 %{3:6} }, { MT_BINARIZE_8 %{3:6} }, MT_BINARIZE_ENDLINE
   
   RETURN
%ENDMACRO

%MACRO MT_BINARIZE 6-*
   %DEFINE %%NAME %1
   %DEFINE %%ARGS %{2:5}
   %REP %0 - 5
      MT_BINARIZE_COMMON %6, %%NAME, %%ARGS
      %ROTATE 1
   %ENDREP
%ENDMACRO   

MT_BINARIZE lower          , ASM_NOP, paddb  , pcmpgtb, ASM_NOP   , mmx, isse, 3dnow, sse2, asse2
MT_BINARIZE upper          , ASM_NOP, psubusb, pcmpeqb, ASM_NOP   , mmx, isse, 3dnow, sse2, asse2
MT_BINARIZE binarize_x_0   , mova   , paddb  , pcmpgtb, MT_AND_1_2, mmx, isse, 3dnow, sse2, asse2
MT_BINARIZE binarize_0_x   , mova   , psubusb, pcmpeqb, MT_AND_1_2, mmx, isse, 3dnow, sse2, asse2
MT_BINARIZE binarize_t_0   , mova   , paddb  , pcmpgtb, MT_AND_1_3, mmx, isse, 3dnow, sse2, asse2
MT_BINARIZE binarize_0_t   , mova   , psubusb, pcmpeqb, MT_AND_1_3, mmx, isse, 3dnow, sse2, asse2
MT_BINARIZE binarize_255_x , mova   , paddb  , pcmpgtb, MT_OR_1_2 , mmx, isse, 3dnow, sse2, asse2
MT_BINARIZE binarize_x_255 , mova   , psubusb, pcmpeqb, MT_OR_1_2 , mmx, isse, 3dnow, sse2, asse2
MT_BINARIZE binarize_255_t , mova   , paddb  , pcmpgtb, MT_OR_1_3 , mmx, isse, 3dnow, sse2, asse2
MT_BINARIZE binarize_t_255 , mova   , psubusb, pcmpeqb, MT_OR_1_3 , mmx, isse, 3dnow, sse2, asse2
MT_BINARIZE binarize_x_t   , ASM_NOP, pmaxub , ASM_NOP, ASM_NOP   , isse, 3dnow, sse2, asse2
MT_BINARIZE binarize_t_x   , ASM_NOP, pminub , ASM_NOP, ASM_NOP   , isse, 3dnow, sse2, asse2




