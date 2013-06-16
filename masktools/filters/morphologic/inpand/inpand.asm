%INCLUDE "morphologic.asm"

;=============================================================================
; Code
;=============================================================================

SECTION .text

   mt_mangle Inpand, square_inpand8_isse
   mt_mangle Inpand, square_inpand8_3dnow
   mt_mangle Inpand, square_inpand8_sse2
   mt_mangle Inpand, square_inpand8_asse2

   mt_mangle Inpand, horizontal_inpand8_isse
   mt_mangle Inpand, horizontal_inpand8_3dnow
   mt_mangle Inpand, horizontal_inpand8_sse2
   mt_mangle Inpand, horizontal_inpand8_asse2

   mt_mangle Inpand, vertical_inpand8_isse
   mt_mangle Inpand, vertical_inpand8_3dnow
   mt_mangle Inpand, vertical_inpand8_sse2
   mt_mangle Inpand, vertical_inpand8_asse2
   
   mt_mangle Inpand, both_inpand8_isse
   mt_mangle Inpand, both_inpand8_3dnow
   mt_mangle Inpand, both_inpand8_sse2
   mt_mangle Inpand, both_inpand8_asse2
   
%MACRO MT_SQUARE_INPAND_64 8
   MT_MORPHOLOGIC_64 %{1:8}, MT_SQUARE_XXPAND_CORE, MT_INPAND_OPERATORS
%ENDMACRO

%MACRO MT_SQUARE_INPAND_8 8
   MT_MORPHOLOGIC_8  %{1:8}, MT_SQUARE_XXPAND_CORE, MT_INPAND_OPERATORS
%ENDMACRO

%MACRO MT_HORIZONTAL_INPAND_64 8
   MT_MORPHOLOGIC_64 %{1:8}, MT_HORIZONTAL_XXPAND_CORE, MT_INPAND_OPERATORS
%ENDMACRO

%MACRO MT_HORIZONTAL_INPAND_8 8
   MT_MORPHOLOGIC_8  %{1:8}, MT_HORIZONTAL_XXPAND_CORE, MT_INPAND_OPERATORS
%ENDMACRO

%MACRO MT_VERTICAL_INPAND_64 8
   MT_MORPHOLOGIC_64 %{1:8}, MT_VERTICAL_XXPAND_CORE, MT_INPAND_OPERATORS
%ENDMACRO

%MACRO MT_VERTICAL_INPAND_8 8
   MT_MORPHOLOGIC_8  %{1:8}, MT_VERTICAL_XXPAND_CORE, MT_INPAND_OPERATORS
%ENDMACRO

%MACRO MT_BOTH_INPAND_64 8
   MT_MORPHOLOGIC_64 %{1:8}, MT_BOTH_XXPAND_CORE, MT_INPAND_OPERATORS
%ENDMACRO

%MACRO MT_BOTH_INPAND_8 8
   MT_MORPHOLOGIC_8  %{1:8}, MT_BOTH_XXPAND_CORE, MT_INPAND_OPERATORS
%ENDMACRO
   
MT_MORPHOLOGIC_FUNCTION isse, square_inpand, MT_SQUARE_INPAND_64, MT_SQUARE_INPAND_8
MT_MORPHOLOGIC_FUNCTION 3dnow, square_inpand, MT_SQUARE_INPAND_64, MT_SQUARE_INPAND_8
MT_MORPHOLOGIC_FUNCTION sse2, square_inpand, MT_SQUARE_INPAND_64, MT_SQUARE_INPAND_8
MT_MORPHOLOGIC_FUNCTION asse2, square_inpand, MT_SQUARE_INPAND_64, MT_SQUARE_INPAND_8

MT_MORPHOLOGIC_FUNCTION isse, horizontal_inpand, MT_HORIZONTAL_INPAND_64, MT_HORIZONTAL_INPAND_8
MT_MORPHOLOGIC_FUNCTION 3dnow, horizontal_inpand, MT_HORIZONTAL_INPAND_64, MT_HORIZONTAL_INPAND_8
MT_MORPHOLOGIC_FUNCTION sse2, horizontal_inpand, MT_HORIZONTAL_INPAND_64, MT_HORIZONTAL_INPAND_8
MT_MORPHOLOGIC_FUNCTION asse2, horizontal_inpand, MT_HORIZONTAL_INPAND_64, MT_HORIZONTAL_INPAND_8

MT_MORPHOLOGIC_FUNCTION isse, vertical_inpand, MT_VERTICAL_INPAND_64, MT_VERTICAL_INPAND_8
MT_MORPHOLOGIC_FUNCTION 3dnow, vertical_inpand, MT_VERTICAL_INPAND_64, MT_VERTICAL_INPAND_8
MT_MORPHOLOGIC_FUNCTION sse2, vertical_inpand, MT_VERTICAL_INPAND_64, MT_VERTICAL_INPAND_8
MT_MORPHOLOGIC_FUNCTION asse2, vertical_inpand, MT_VERTICAL_INPAND_64, MT_VERTICAL_INPAND_8

MT_MORPHOLOGIC_FUNCTION isse, both_inpand, MT_BOTH_INPAND_64, MT_BOTH_INPAND_8
MT_MORPHOLOGIC_FUNCTION 3dnow, both_inpand, MT_BOTH_INPAND_64, MT_BOTH_INPAND_8
MT_MORPHOLOGIC_FUNCTION sse2, both_inpand, MT_BOTH_INPAND_64, MT_BOTH_INPAND_8
MT_MORPHOLOGIC_FUNCTION asse2, both_inpand, MT_BOTH_INPAND_64, MT_BOTH_INPAND_8
