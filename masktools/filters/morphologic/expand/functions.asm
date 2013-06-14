%INCLUDE "morphologic.asm"

;=============================================================================
; Code
;=============================================================================

SECTION .text

   mt_mangle Expand, square_expand8_isse
   mt_mangle Expand, square_expand8_3dnow
   mt_mangle Expand, square_expand8_sse2
   mt_mangle Expand, square_expand8_asse2

   mt_mangle Expand, horizontal_expand8_isse
   mt_mangle Expand, horizontal_expand8_3dnow
   mt_mangle Expand, horizontal_expand8_sse2
   mt_mangle Expand, horizontal_expand8_asse2

   mt_mangle Expand, vertical_expand8_isse
   mt_mangle Expand, vertical_expand8_3dnow
   mt_mangle Expand, vertical_expand8_sse2
   mt_mangle Expand, vertical_expand8_asse2
   
   mt_mangle Expand, both_expand8_isse
   mt_mangle Expand, both_expand8_3dnow
   mt_mangle Expand, both_expand8_sse2
   mt_mangle Expand, both_expand8_asse2
   
%MACRO MT_SQUARE_EXPAND_64 8
   MT_MORPHOLOGIC_64 %{1:8}, MT_SQUARE_XXPAND_CORE, MT_EXPAND_OPERATORS
%ENDMACRO

%MACRO MT_SQUARE_EXPAND_8 8
   MT_MORPHOLOGIC_8  %{1:8}, MT_SQUARE_XXPAND_CORE, MT_EXPAND_OPERATORS
%ENDMACRO

%MACRO MT_HORIZONTAL_EXPAND_64 8
   MT_MORPHOLOGIC_64 %{1:8}, MT_HORIZONTAL_XXPAND_CORE, MT_EXPAND_OPERATORS
%ENDMACRO

%MACRO MT_HORIZONTAL_EXPAND_8 8
   MT_MORPHOLOGIC_8  %{1:8}, MT_HORIZONTAL_XXPAND_CORE, MT_EXPAND_OPERATORS
%ENDMACRO

%MACRO MT_VERTICAL_EXPAND_64 8
   MT_MORPHOLOGIC_64 %{1:8}, MT_VERTICAL_XXPAND_CORE, MT_EXPAND_OPERATORS
%ENDMACRO

%MACRO MT_VERTICAL_EXPAND_8 8
   MT_MORPHOLOGIC_8  %{1:8}, MT_VERTICAL_XXPAND_CORE, MT_EXPAND_OPERATORS
%ENDMACRO

%MACRO MT_BOTH_EXPAND_64 8
   MT_MORPHOLOGIC_64 %{1:8}, MT_BOTH_XXPAND_CORE, MT_EXPAND_OPERATORS
%ENDMACRO

%MACRO MT_BOTH_EXPAND_8 8
   MT_MORPHOLOGIC_8  %{1:8}, MT_BOTH_XXPAND_CORE, MT_EXPAND_OPERATORS
%ENDMACRO
   
MT_MORPHOLOGIC_FUNCTION isse, square_expand, MT_SQUARE_EXPAND_64, MT_SQUARE_EXPAND_8
MT_MORPHOLOGIC_FUNCTION 3dnow, square_expand, MT_SQUARE_EXPAND_64, MT_SQUARE_EXPAND_8
MT_MORPHOLOGIC_FUNCTION sse2, square_expand, MT_SQUARE_EXPAND_64, MT_SQUARE_EXPAND_8
MT_MORPHOLOGIC_FUNCTION asse2, square_expand, MT_SQUARE_EXPAND_64, MT_SQUARE_EXPAND_8

MT_MORPHOLOGIC_FUNCTION isse, horizontal_expand, MT_HORIZONTAL_EXPAND_64, MT_HORIZONTAL_EXPAND_8
MT_MORPHOLOGIC_FUNCTION 3dnow, horizontal_expand, MT_HORIZONTAL_EXPAND_64, MT_HORIZONTAL_EXPAND_8
MT_MORPHOLOGIC_FUNCTION sse2, horizontal_expand, MT_HORIZONTAL_EXPAND_64, MT_HORIZONTAL_EXPAND_8
MT_MORPHOLOGIC_FUNCTION asse2, horizontal_expand, MT_HORIZONTAL_EXPAND_64, MT_HORIZONTAL_EXPAND_8

MT_MORPHOLOGIC_FUNCTION isse, vertical_expand, MT_VERTICAL_EXPAND_64, MT_VERTICAL_EXPAND_8
MT_MORPHOLOGIC_FUNCTION 3dnow, vertical_expand, MT_VERTICAL_EXPAND_64, MT_VERTICAL_EXPAND_8
MT_MORPHOLOGIC_FUNCTION sse2, vertical_expand, MT_VERTICAL_EXPAND_64, MT_VERTICAL_EXPAND_8
MT_MORPHOLOGIC_FUNCTION asse2, vertical_expand, MT_VERTICAL_EXPAND_64, MT_VERTICAL_EXPAND_8

MT_MORPHOLOGIC_FUNCTION isse, both_expand, MT_BOTH_EXPAND_64, MT_BOTH_EXPAND_8
MT_MORPHOLOGIC_FUNCTION 3dnow, both_expand, MT_BOTH_EXPAND_64, MT_BOTH_EXPAND_8
MT_MORPHOLOGIC_FUNCTION sse2, both_expand, MT_BOTH_EXPAND_64, MT_BOTH_EXPAND_8
MT_MORPHOLOGIC_FUNCTION asse2, both_expand, MT_BOTH_EXPAND_64, MT_BOTH_EXPAND_8
