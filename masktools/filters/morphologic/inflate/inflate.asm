%INCLUDE "morphologic.asm"

;=============================================================================
; Code
;=============================================================================

SECTION .text

   mt_mangle Inflate, inflate8_isse
   mt_mangle Inflate, inflate8_3dnow
   mt_mangle Inflate, inflate8_sse2
   mt_mangle Inflate, inflate8_asse2

%MACRO MT_INFLATE_64 8
   MT_MORPHOLOGIC_64 %{1:8}, MT_SQUARE_XXFLATE_CORE, MT_INFLATE_OPERATORS
%ENDMACRO

%MACRO MT_INFLATE_8 8
   MT_MORPHOLOGIC_8  %{1:8}, MT_SQUARE_XXFLATE_CORE, MT_INFLATE_OPERATORS
%ENDMACRO

MT_MORPHOLOGIC_FUNCTION isse, inflate, MT_INFLATE_64, MT_INFLATE_8
MT_MORPHOLOGIC_FUNCTION 3dnow, inflate, MT_INFLATE_64, MT_INFLATE_8
MT_MORPHOLOGIC_FUNCTION sse2, inflate, MT_INFLATE_64, MT_INFLATE_8
MT_MORPHOLOGIC_FUNCTION asse2, inflate, MT_INFLATE_64, MT_INFLATE_8
