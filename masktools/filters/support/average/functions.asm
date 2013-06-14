%INCLUDE "support.asm"

;=============================================================================
; Code
;=============================================================================

SECTION .text

mt_mangle Average, average8_isse
mt_mangle Average, average8_3dnow
mt_mangle Average, average8_sse2
mt_mangle Average, average8_asse2
   
%MACRO MT_AVERAGE_8 0

   ASM_START_MMX

   movau          m0, [pbDestination + xax]
   
   %IFIDNI movau, mova
      pavgb       m0, [pbSource + xax]
   %ELSE
      movu        m1, [pbSource + xax]
      pavgb       m0, m1
   %ENDIF
   
   movau          [pbDestination + xax], m0
   
   ASM_STOP_MMX
   
%ENDMACRO

%MACRO MT_AVERAGE_64 0

   prefetchrw     [pbDestination + xax + 256]
   prefetchr      [pbSource      + xax + 256]

   %ASSIGN offset 0
   %REP 64 / (4 * mfull)
      
      movau       m0, [pbDestination + xax + offset + 0 * mfull]
      movau       m1, [pbDestination + xax + offset + 1 * mfull]
      movau       m2, [pbDestination + xax + offset + 2 * mfull]
      movau       m3, [pbDestination + xax + offset + 3 * mfull]
      
      %IFIDNI movau, mova
         pavgb    m0, [pbSource      + xax + offset + 0 * mfull]
         pavgb    m1, [pbSource      + xax + offset + 1 * mfull]
         pavgb    m2, [pbSource      + xax + offset + 2 * mfull]
         pavgb    m3, [pbSource      + xax + offset + 3 * mfull]
      %ELSE   
         movau    m4, [pbSource      + xax + offset + 0 * mfull]
         movau    m5, [pbSource      + xax + offset + 1 * mfull]
         movau    m6, [pbSource      + xax + offset + 2 * mfull]
         movau    m7, [pbSource      + xax + offset + 3 * mfull]
         pavgb    m0, m4
         pavgb    m1, m5
         pavgb    m2, m6
         pavgb    m3, m7
      %ENDIF   

      movau       [pbDestination + xax + offset + 0 * mfull], m0
      movau       [pbDestination + xax + offset + 1 * mfull], m1
      movau       [pbDestination + xax + offset + 2 * mfull], m2
      movau       [pbDestination + xax + offset + 3 * mfull], m3
      
      %ASSIGN offset offset + 4 * mfull
      
   %ENDREP

%ENDMACRO

%MACRO MT_AVERAGE_ENDLINE 0
   add                  pbDestination, nDestinationPitch
   add                  pbSource, nSourcePitch
%ENDMACRO

MT_SUPPORT_COMMON    isse, average, AVERAGE
MT_SUPPORT_COMMON    3dnow, average, AVERAGE
MT_SUPPORT_COMMON    sse2, average, AVERAGE
MT_SUPPORT_COMMON    asse2, average, AVERAGE
