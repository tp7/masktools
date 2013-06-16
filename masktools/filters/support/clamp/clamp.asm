%INCLUDE "common.asm"

;=============================================================================
; Code
;=============================================================================

SECTION .text

mt_mangle Clamp, clamp8_isse
mt_mangle Clamp, clamp8_3dnow
mt_mangle Clamp, clamp8_sse2
mt_mangle Clamp, clamp8_asse2
   
%MACRO MT_CLAMP_8 0

   ASM_START_MMX

   movau          m0, [pbSource1     + xax]
   movau          m1, [pbSource2     + xax]

   paddusb        m0, m6             ; overshoot
   psubusb        m1, m7             ; undershoot

   %IFIDNI mova, movau
      pminub      m0, [pbDestination + xax]
   %ELSE
      movau       m2, [pbDestination + xax]
      pminub      m0, m2
   %ENDIF
      
   pmaxub      m0, m1
   
   movau       [pbDestination + xax], m0
   
   ASM_STOP_MMX
   
%ENDMACRO

%MACRO MT_CLAMP_64 0

   prefetchrw     [pbDestination + xax + 64]
   prefetchr      [pbSource1     + xax + 64]
   prefetchr      [pbSource2     + xax + 64]
   
   %ASSIGN offset 0
   %REP 64 / (mfull * 2)

      movau       m0, [pbSource1     + xax + offset + 0 * mfull]
      movau       m1, [pbSource1     + xax + offset + 1 * mfull]
      movau       m2, [pbSource2     + xax + offset + 0 * mfull]
      movau       m3, [pbSource2     + xax + offset + 1 * mfull]
   
      paddusb     m0, m6
      paddusb     m1, m6
      psubusb     m2, m7
      psubusb     m3, m7
      
      %IFIDNI mova, movau
   
         pminub   m0, [pbDestination + xax + offset + 0 * mfull]
         pminub   m1, [pbDestination + xax + offset + 1 * mfull]
         
      %ELSE

         movau    m4, [pbDestination + xax + offset + 0 * mfull]
         movau    m5, [pbDestination + xax + offset + 1 * mfull]

         pminub   m0, m4
         pminub   m1, m5
         
      %ENDIF   
      
      pmaxub      m0, m2
      pmaxub      m1, m3
   
      movau       [pbDestination + xax + offset + 0 * mfull], m0
      movau       [pbDestination + xax + offset + 1 * mfull], m1

      %ASSIGN offset offset + mfull * 2
      
   %ENDREP   
   
%ENDMACRO

%MACRO MT_CLAMP_ENDLINE 0
   add            pbDestination, nDestinationPitch
   add            pbSource1, nSource1Pitch
   add            pbSource2, nSource2Pitch
%ENDMACRO

%MACRO MT_CLAMP_COMMON  2
%2 %+ 8_ %+ %1:
   STACK          xax, xdx,, pbDestination, *, nDestinationPitch, pbSource1, *, nSource1Pitch, pbSource2, *, nSource2Pitch, nWidth, *, nHeight, *, nOvershoot, nUndershoot
   ASM_XMMX       %1
   
   movd           m6, nOvershoot
   movd           m7, nUndershoot
   psplatbr       m6
   psplatbr       m7
   
   ASM_START_MMX
   
   movd           m6, nOvershoot
   movd           m7, nUndershoot
   psplatbr       m6
   psplatbr       m7
   
   ASM_STOP_MMX
   
   MT_UNROLL_INPLACE_WIDTH    MT_CLAMP_64, MT_CLAMP_8, MT_CLAMP_ENDLINE
      
   RETURN
%ENDMACRO

MT_CLAMP_COMMON      isse, clamp
MT_CLAMP_COMMON      3dnow, clamp
MT_CLAMP_COMMON      sse2, clamp
MT_CLAMP_COMMON      asse2, clamp
