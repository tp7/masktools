%INCLUDE "support.asm"

;=============================================================================
; Code
;=============================================================================

SECTION .text

mt_mangle AddDiff, adddiff8_mmx
mt_mangle AddDiff, adddiff8_isse
mt_mangle AddDiff, adddiff8_3dnow
mt_mangle AddDiff, adddiff8_sse2
mt_mangle AddDiff, adddiff8_asse2
   
%MACRO MT_ADDDIFF_8     0
   MT_SUPPORT_DIFF_8    paddsb
%ENDMACRO

%MACRO MT_ADDDIFF_64    0
   MT_SUPPORT_DIFF_64   paddsb
%ENDMACRO

%MACRO MT_ADDDIFF_ENDLINE 0
   add                  pbDestination, nDestinationPitch
   add                  pbSource, nSourcePitch
%ENDMACRO

MT_SUPPORT_COMMON    mmx, adddiff, ADDDIFF
MT_SUPPORT_COMMON    isse, adddiff, ADDDIFF
MT_SUPPORT_COMMON    3dnow, adddiff, ADDDIFF
MT_SUPPORT_COMMON    sse2, adddiff, ADDDIFF
MT_SUPPORT_COMMON    asse2, adddiff, ADDDIFF
