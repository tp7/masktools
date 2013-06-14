; -----------------------------------------------------------------------------
; This header defines a set of often used simd constant. All those constants
; are 16 bits aligned, and comes in byte/word/dword/qword flavors.
; Some of them are specifically designed for specific shuffle operations,
; used by some macros defining pseudo instructions
; -----------------------------------------------------------------------------

ASM_SECTION_RODATA
ALIGN 16

; -----------------------------------------------------------------------------
	
%MACRO ASM_DEFINE_CONSTANT 4

   %ASSIGN %%TEMP_VALUE %4

ASM_%1_ %+ %%TEMP_VALUE:
ASM_%1_%4:
   TIMES %3 %2 %4
%ENDMACRO   

; -----------------------------------------------------------------------------

%MACRO ASM_DEFINE_CONSTANTS 1

   %IF 0x%1 <= 0xFF

      ASM_DEFINE_CONSTANT QWORD, DQ,  2, 0x00000000000000%1
      ASM_DEFINE_CONSTANT DWORD, DD,  4, 0x000000%1
      ASM_DEFINE_CONSTANT  WORD, DW,  8, 0x00%1
      ASM_DEFINE_CONSTANT  BYTE, DB, 16, 0x%1

   %ELIF 0x%1 <= 0xFFFF

      ASM_DEFINE_CONSTANT QWORD, DQ,  2, 0x000000000000%1
      ASM_DEFINE_CONSTANT DWORD, DD,  4, 0x0000%1
      ASM_DEFINE_CONSTANT  WORD, DW,  8, 0x%1

   %ELIF 0x%1 <= 0xFFFFFFFF

      ASM_DEFINE_CONSTANT QWORD, DQ,  2, 0x00000000%1
      ASM_DEFINE_CONSTANT DWORD, DD,  4, 0x%1

   %ELIF 0x%1 <= 0xFFFFFFFFFFFFFFFF

      ASM_DEFINE_CONSTANT QWORD, DQ,  2, 0x%1

   %ENDIF

%ENDMACRO   

; -----------------------------------------------------------------------------

ASM_DEFINE_CONSTANTS 00
ASM_DEFINE_CONSTANTS 01
ASM_DEFINE_CONSTANTS 02
ASM_DEFINE_CONSTANTS 03
ASM_DEFINE_CONSTANTS 04
ASM_DEFINE_CONSTANTS 07
ASM_DEFINE_CONSTANTS 08
ASM_DEFINE_CONSTANTS 0F
ASM_DEFINE_CONSTANTS 10
ASM_DEFINE_CONSTANTS 1F
ASM_DEFINE_CONSTANTS 20
ASM_DEFINE_CONSTANTS 3F
ASM_DEFINE_CONSTANTS 40
ASM_DEFINE_CONSTANTS 7F
ASM_DEFINE_CONSTANTS 80

; -----------------------------------------------------------------------------

ASM_DEFINE_CONSTANTS 0100
ASM_DEFINE_CONSTANTS 01FF
ASM_DEFINE_CONSTANTS 0200
ASM_DEFINE_CONSTANTS 03FF
ASM_DEFINE_CONSTANTS 0400
ASM_DEFINE_CONSTANTS 07FF
ASM_DEFINE_CONSTANTS 0800
ASM_DEFINE_CONSTANTS 0FFF
ASM_DEFINE_CONSTANTS 1000
ASM_DEFINE_CONSTANTS 1FFF
ASM_DEFINE_CONSTANTS 2000
ASM_DEFINE_CONSTANTS 3FFF
ASM_DEFINE_CONSTANTS 4000
ASM_DEFINE_CONSTANTS 7FFF
ASM_DEFINE_CONSTANTS 8000

; -----------------------------------------------------------------------------

ASM_DEFINE_CONSTANTS 00010000

; -----------------------------------------------------------------------------

ASM_DEFINE_CONSTANT QWORD, DQ,  2, 0x00000000000000FF
ASM_DEFINE_CONSTANT DWORD, DD,  4, 0x000000FF
ASM_DEFINE_CONSTANT  WORD, DW,  8, 0x00FF
ASM_DEFINE_CONSTANT  BYTE, DB, 16, 0xFF

ASM_DEFINE_CONSTANT QWORD, DQ,  2, 0x000000000000FFFF
ASM_DEFINE_CONSTANT DWORD, DD,  4, 0x0000FFFF
ASM_DEFINE_CONSTANT  WORD, DW,  8, 0xFFFF

ASM_DEFINE_CONSTANT QWORD, DQ,  2, 0x00000000FFFFFFFF
ASM_DEFINE_CONSTANT DWORD, DD,  4, 0xFFFFFFFF

ASM_DEFINE_CONSTANT QWORD, DQ,  2, 0xFFFFFFFFFFFFFFFF

; -----------------------------------------------------------------------------

ASM_ZERO:
	TIMES 16 DB 0x00
ATUTILS_SPLAT_BYTE_DWORD:
   DB 0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x04, 0x04, 0x08, 0x08, 0x08, 0x08, 0x0C, 0x0C, 0x0C, 0x0C
ATUTILS_SPLAT_BYTE_DQWORD:
   DB 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
ATUTILS_SPLAT_WORD_DQWORD:
   DB 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01
ATUTILS_SPLAT_BYTE_DQWORD_ZWORD:
   DB 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF
ATUTILS_REORDER_BYTE_ODD_EVEN:
   DB 0x00, 0x08, 0x01, 0x09, 0x02, 0x0A, 0x03, 0x0B, 0x04, 0x0C, 0x05, 0x0D, 0x06, 0x0E, 0x07, 0x0F

; -----------------------------------------------------------------------------
