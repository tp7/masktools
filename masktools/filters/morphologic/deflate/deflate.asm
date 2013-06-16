%INCLUDE "morphologic.asm"

;=============================================================================
; Code
;=============================================================================

SECTION .text

   mt_mangle Deflate, deflate8_isse
   mt_mangle Deflate, deflate8_3dnow
   mt_mangle Deflate, deflate8_sse2
   mt_mangle Deflate, deflate8_asse2

%MACRO MT_DEFLATE_64 8
   MT_MORPHOLOGIC_64 %{1:8}, MT_SQUARE_XXFLATE_CORE, MT_DEFLATE_OPERATORS
%ENDMACRO

%MACRO MT_DEFLATE_8 8
   MT_MORPHOLOGIC_8  %{1:8}, MT_SQUARE_XXFLATE_CORE, MT_DEFLATE_OPERATORS
%ENDMACRO

MT_MORPHOLOGIC_FUNCTION isse, deflate, MT_DEFLATE_64, MT_DEFLATE_8
MT_MORPHOLOGIC_FUNCTION 3dnow, deflate, MT_DEFLATE_64, MT_DEFLATE_8
MT_MORPHOLOGIC_FUNCTION sse2, deflate, MT_DEFLATE_64, MT_DEFLATE_8
MT_MORPHOLOGIC_FUNCTION asse2, deflate, MT_DEFLATE_64, MT_DEFLATE_8
