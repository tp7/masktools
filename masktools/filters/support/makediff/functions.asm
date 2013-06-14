%INCLUDE "support.asm"

;=============================================================================
; Code
;=============================================================================

SECTION .text

   mt_mangle MakeDiff, makediff8_mmx
   mt_mangle MakeDiff, makediff8_isse
   mt_mangle MakeDiff, makediff8_3dnow
   mt_mangle MakeDiff, makediff8_sse2
   mt_mangle MakeDiff, makediff8_asse2
   
%MACRO MT_MAKEDIFF_8    0
   MT_SUPPORT_DIFF_8    psubsb
%ENDMACRO

%MACRO MT_MAKEDIFF_64   0
   MT_SUPPORT_DIFF_64   psubsb
%ENDMACRO

%MACRO MT_MAKEDIFF_ENDLINE 0
   add                  pbDestination, nDestinationPitch
   add                  pbSource, nSourcePitch
%ENDMACRO

MT_SUPPORT_COMMON    mmx, makediff, MAKEDIFF
MT_SUPPORT_COMMON    isse, makediff, MAKEDIFF
MT_SUPPORT_COMMON    3dnow, makediff, MAKEDIFF
MT_SUPPORT_COMMON    sse2, makediff, MAKEDIFF
MT_SUPPORT_COMMON    asse2, makediff, MAKEDIFF
