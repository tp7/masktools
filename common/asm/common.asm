%INCLUDE "asm.asm"

;=============================================================================
; Macros
;=============================================================================

%MACRO mt_mangle_single 2
   ASM_GLOBAL %1_%2
   %DEFINE %2 %1_%2
%ENDMACRO   

%MACRO mt_mangle 2-*
   %IF %0 = 2
      mt_mangle_single %1, %2
   %ELSE
      %DEFINE %%NAMESPACE %1
      %DEFINE %%BASE %2
      %REP %0 - 2
         mt_mangle_single %%NAMESPACE, %%BASE %+ _ %+ %3
         %ROTATE 1
      %ENDREP
   %ENDIF
%ENDMACRO   

;=============================================================================
; Constants (no need to redefine ASM_SECTION_RODATA, avoid a dumb warning from yasm)
;=============================================================================

ASM_SECTION_RODATA

   MT_MASK_LEFT_XMM  : DB 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
   MT_MASK_RIGHT_XMM : DB 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF
   MT_MASK_LEFT_MMX  : DB 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
   MT_MASK_RIGHT_MMX : DB 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF

;=============================================================================
; unrolling stuff
;=============================================================================

; uses two temporaries (xdx and xax), and nHeight register
; needs nWidth to be a constant 
; nHeight will be trashed, xax the width counter, and xdx the loop stop
; three parameters : code for the unrolled loop ( 64 bytes )
;                    code for the aligning loops ( head & tail - 8 bytes)
;                    code for increasing pointers
%MACRO MT_UNROLL_INPLACE_WIDTH 3
   mov                  xdxd, nWidthd          ; save it into xdx ( unrolled loop stop )
   and                  xdxd, ~0x3F           ; make the loop 64-bytes aligned
   
%%loopy
   
   xor                  xax, xax          ; reset xax ( loop counter )
   
   cmp                  xaxd, xdxd          ; check that we can do 64 pixels 
   je                   %%no64
   
%%loop

   %1                                     ; unrolled code
   
   add                  xax, 64
   cmp                  xaxd, xdxd
   jl                   %%loop
   
%%no64   
   
   cmp                  xaxd, nWidthd
   je                   %%end

%%tail

   %2                                     ; tail code ( same as head )

   add                  xax, 8
   cmp                  xaxd, nWidthd
   jl                   %%tail
   
%%end

   %3                                     ; change line code
   sub                  nHeightd, 1
   jnz                  %%loopy
   
%ENDMACRO   
   
;=============================================================================
; convolution & border stuff
;=============================================================================

%MACRO MT_LOAD_PADDED         4-6

   %IF %3 = 1
      %IF ASM_HAS_MMX_REGISTERS
         %DEFINE MT_MASK MT_MASK_RIGHT_MMX
      %ELSE
         %DEFINE MT_MASK MT_MASK_RIGHT_XMM
      %ENDIF
   %ELSE
      %IF ASM_HAS_MMX_REGISTERS
         %DEFINE MT_MASK MT_MASK_LEFT_MMX
      %ELSE
         %DEFINE MT_MASK MT_MASK_LEFT_XMM
      %ENDIF
   %ENDIF

   %IF %0 = 6
      movu                    %6, [%2]
      mova                    %4, %6
      pshift                  %6, -(%3)
      pand                    %4, [MT_MASK]
      por                     %6, %4
      %5                      %1, %6
   %ELSE
      movu                    %1, [%2]
      mova                    %4, %1
      pshift                  %1, -(%3)
      pand                    %4, [MT_MASK]
      por                     %1, %4
   %ENDIF
%ENDMACRO  

%MACRO MT_LOAD_NORMAL         4-6
   %IF %0 = 6
      %IFIDNI %3, aligned
         %5                   %1, [%2]
      %ELIFIDNI movau, mova
         %5                   %1, [%2]
      %ELSE
         movu                 %6, [%2]
         %5                   %1, %6
      %ENDIF
   %ELSE
      ifidni %3, aligned, { movau %1, [%2] }, else, { movu %1, [%2] }
   %ENDIF
%ENDMACRO
   
;=============================================================================
;left border
;=============================================================================

%MACRO MT_LOAD_LEFT_BORDER    4-6
   MT_LOAD_PADDED             %1, %2 + %3, -1, %{4:0}
%ENDMACRO

%MACRO MT_LOAD_LEFT_NORMAL    4-6
   MT_LOAD_NORMAL             %1, %2 + %3 - 1, unaligned, %{4:0}
%ENDMACRO

;=============================================================================
;right border
;=============================================================================

%MACRO MT_LOAD_RIGHT_BORDER   4-6
   MT_LOAD_PADDED             %1, %2 + %3, +1, %{4:0}
%ENDMACRO

%MACRO MT_LOAD_RIGHT_NORMAL   4-6
   MT_LOAD_NORMAL             %1, %2 + %3 + 1, unaligned, %{4:0}
%ENDMACRO

;=============================================================================
; top border
;=============================================================================

%MACRO MT_LOAD_UP_BORDER      4-6
   MT_LOAD_NORMAL             %1, %2 + %3, aligned, %{4:0}
%ENDMACRO

%MACRO MT_LOAD_UP_NORMAL      4-6
   MT_LOAD_NORMAL             %1, %2, aligned, %{4:0}
%ENDMACRO

;=============================================================================
; bottom border
;=============================================================================

%MACRO MT_LOAD_DOWN_BORDER    4-6
   MT_LOAD_NORMAL             %1, %2 + %3, aligned, %{4:0}
%ENDMACRO

%MACRO MT_LOAD_DOWN_NORMAL    4-6
   MT_LOAD_NORMAL             %1, %2 + 2 * %3, aligned, %{4:0}
%ENDMACRO

;=============================================================================
; top left
;=============================================================================

%MACRO MT_LOAD_UP_LEFT_BORDER_UP_LEFT 4-6
   MT_LOAD_PADDED             %1, %2 + %3, -1, %{4:0}
%ENDMACRO

%MACRO MT_LOAD_UP_LEFT_BORDER_LEFT 4-6
   MT_LOAD_PADDED             %1, %2, -1, %{4:0}
%ENDMACRO

%MACRO MT_LOAD_UP_LEFT_BORDER_UP 4-6
   MT_LOAD_NORMAL             %1, %2 + %3 - 1, unaligned, %{4:0}
%ENDMACRO

%MACRO MT_LOAD_UP_LEFT_NORMAL 4-6
   MT_LOAD_NORMAL             %1, %2 - 1, unaligned, %{4:0}
%ENDMACRO

;=============================================================================
; top right
;=============================================================================

%MACRO MT_LOAD_UP_RIGHT_BORDER_UP_RIGHT 4-6
   MT_LOAD_PADDED             %1, %2 + %3, +1, %{4:0}
%ENDMACRO

%MACRO MT_LOAD_UP_RIGHT_BORDER_RIGHT 4-6
   MT_LOAD_PADDED             %1, %2, +1, %{4:0}
%ENDMACRO

%MACRO MT_LOAD_UP_RIGHT_BORDER_UP 4-6
   MT_LOAD_NORMAL             %1, %2 + %3 + 1, unaligned, %{4:0}
%ENDMACRO

%MACRO MT_LOAD_UP_RIGHT_NORMAL 4-6
   MT_LOAD_NORMAL             %1, %2 + 1, unaligned, %{4:0}
%ENDMACRO

;=============================================================================
; bottom left
;=============================================================================

%MACRO MT_LOAD_DOWN_LEFT_BORDER_DOWN_LEFT 4-6
   MT_LOAD_PADDED             %1, %2 + %3, -1, %{4:0}
%ENDMACRO

%MACRO MT_LOAD_DOWN_LEFT_BORDER_LEFT 4-6
   MT_LOAD_PADDED             %1, %2 + 2 * %3, -1, %{4:0}
%ENDMACRO

%MACRO MT_LOAD_DOWN_LEFT_BORDER_DOWN 4-6
   MT_LOAD_NORMAL             %1, %2 + %3 - 1, unaligned, %{4:0}
%ENDMACRO

%MACRO MT_LOAD_DOWN_LEFT_NORMAL 4-6
   MT_LOAD_NORMAL             %1, %2 + 2 * %3 - 1, unaligned, %{4:0}
%ENDMACRO

;=============================================================================
; bottom right
;=============================================================================

%MACRO MT_LOAD_DOWN_RIGHT_BORDER_DOWN_RIGHT 4-6
   MT_LOAD_PADDED             %1, %2 + %3, +1, %{4:0}
%ENDMACRO

%MACRO MT_LOAD_DOWN_RIGHT_BORDER_RIGHT 4-6
   MT_LOAD_PADDED             %1, %2 + 2 * %3, +1, %{4:0}
%ENDMACRO

%MACRO MT_LOAD_DOWN_RIGHT_BORDER_DOWN 4-6
   MT_LOAD_NORMAL             %1, %2 + %3 + 1, unaligned, %{4:0}
%ENDMACRO

%MACRO MT_LOAD_DOWN_RIGHT_NORMAL 4-6
   MT_LOAD_NORMAL             %1, %2 + 2 * %3 + 1, unaligned, %{4:0}
%ENDMACRO

; combos

%DEFINE MT_TOP_LEFT     MT_LOAD_UP_LEFT_BORDER_UP_LEFT, MT_LOAD_UP_BORDER, MT_LOAD_UP_RIGHT_BORDER_UP      , MT_LOAD_LEFT_BORDER, MT_LOAD_RIGHT_NORMAL, MT_LOAD_DOWN_LEFT_BORDER_LEFT     , MT_LOAD_DOWN_NORMAL, MT_LOAD_DOWN_RIGHT_NORMAL
%DEFINE MT_TOP          MT_LOAD_UP_LEFT_BORDER_UP     , MT_LOAD_UP_BORDER, MT_LOAD_UP_RIGHT_BORDER_UP      , MT_LOAD_LEFT_NORMAL, MT_LOAD_RIGHT_NORMAL, MT_LOAD_DOWN_LEFT_NORMAL          , MT_LOAD_DOWN_NORMAL, MT_LOAD_DOWN_RIGHT_NORMAL
%DEFINE MT_TOP_RIGHT    MT_LOAD_UP_LEFT_BORDER_UP     , MT_LOAD_UP_BORDER, MT_LOAD_UP_RIGHT_BORDER_UP_RIGHT, MT_LOAD_LEFT_NORMAL, MT_LOAD_RIGHT_BORDER, MT_LOAD_DOWN_LEFT_NORMAL          , MT_LOAD_DOWN_NORMAL, MT_LOAD_DOWN_RIGHT_BORDER_RIGHT

%DEFINE MT_LEFT         MT_LOAD_UP_LEFT_BORDER_LEFT   , MT_LOAD_UP_NORMAL, MT_LOAD_UP_RIGHT_NORMAL         , MT_LOAD_LEFT_BORDER, MT_LOAD_RIGHT_NORMAL, MT_LOAD_DOWN_LEFT_BORDER_LEFT     , MT_LOAD_DOWN_NORMAL, MT_LOAD_DOWN_RIGHT_NORMAL
%DEFINE MT_NORMAL       MT_LOAD_UP_LEFT_NORMAL        , MT_LOAD_UP_NORMAL, MT_LOAD_UP_RIGHT_NORMAL         , MT_LOAD_LEFT_NORMAL, MT_LOAD_RIGHT_NORMAL, MT_LOAD_DOWN_LEFT_NORMAL          , MT_LOAD_DOWN_NORMAL, MT_LOAD_DOWN_RIGHT_NORMAL
%DEFINE MT_RIGHT        MT_LOAD_UP_LEFT_NORMAL        , MT_LOAD_UP_NORMAL, MT_LOAD_UP_RIGHT_BORDER_RIGHT   , MT_LOAD_LEFT_NORMAL, MT_LOAD_RIGHT_BORDER, MT_LOAD_DOWN_LEFT_NORMAL          , MT_LOAD_DOWN_NORMAL, MT_LOAD_DOWN_RIGHT_BORDER_RIGHT

%DEFINE MT_BOTTOM_LEFT  MT_LOAD_UP_LEFT_BORDER_LEFT   , MT_LOAD_UP_NORMAL, MT_LOAD_UP_RIGHT_NORMAL         , MT_LOAD_LEFT_BORDER, MT_LOAD_RIGHT_NORMAL, MT_LOAD_DOWN_LEFT_BORDER_DOWN_LEFT, MT_LOAD_DOWN_BORDER, MT_LOAD_DOWN_RIGHT_BORDER_DOWN
%DEFINE MT_BOTTOM       MT_LOAD_UP_LEFT_NORMAL        , MT_LOAD_UP_NORMAL, MT_LOAD_UP_RIGHT_NORMAL         , MT_LOAD_LEFT_NORMAL, MT_LOAD_RIGHT_NORMAL, MT_LOAD_DOWN_LEFT_BORDER_DOWN     , MT_LOAD_DOWN_BORDER, MT_LOAD_DOWN_RIGHT_BORDER_DOWN
%DEFINE MT_BOTTOM_RIGHT MT_LOAD_UP_LEFT_NORMAL        , MT_LOAD_UP_NORMAL, MT_LOAD_UP_RIGHT_BORDER_RIGHT   , MT_LOAD_LEFT_NORMAL, MT_LOAD_RIGHT_BORDER, MT_LOAD_DOWN_LEFT_BORDER_DOWN     , MT_LOAD_DOWN_BORDER, MT_LOAD_DOWN_RIGHT_BORDER_DOWN_RIGHT

;=============================================================================
; processing
;=============================================================================

%MACRO MT_PROCESS_LINE     7
   xor                     xax, xax                         ; loop counter resetted
   
   %2                      %3                               ; 8-pixels code, left
   add                     xax, 8
   %6                                                       ; next 8 pixels

   cmp                     xaxd, nUnrolledWidthd
   je                      %%tailxchecked
   
   %2                      %4                               ; 8-pixels code, center
   add                     xax, 8
   %6                                                       ; next 8 pixels

   cmp                     xaxd, nUnrolledWidthd
   je                      %%tailxchecked
   
%%loopx:

   %1                      %4                               ; 64-pixels code, center
   add                     xax, 64
   %7                                                       ; next 64 pixels
   
   cmp                     xaxd, nUnrolledWidthd
   jl                      %%loopx
   
%%tailxchecked
   
   cmp                     xaxd, nTailingWidthd
   je                      %%endx
   
%%tailx:

   %2                      %4                               ; 8-pixels code, center
   add                     xax, 8
   %6                                                       ; next 8 pixels
   
   cmp                     xaxd, nTailingWidthd
   jl                      %%tailx
  
%%endx:   
   
   %2                      %5                               ; 8-pixels, right
   %6                                                       ; next 8 pixels
%ENDMACRO   
   
%MACRO MT_PROCESS_CONVOLUTION 5
   mov                     xaxd, nWidthd
   sub                     xaxd, 8
   mov                     nTailingWidthd, xaxd             ; row length, minus 8 last bytes
   cmp                     xaxd, 16
   jl                      %%nounroll
   sub                     xaxd, 16                         ; row length, minus 16 first bytes
   and                     xaxd, ~0x3F                      ; multiple of 64
   add                     xaxd, 16                         ; add back the 16 first bytes
%%nounroll:   
   mov                     nUnrolledWidthd, xaxd            ; row length, rounded to the last unrolled loop

   ; top line

   MT_PROCESS_LINE         %1, %2, {MT_TOP_LEFT}, {MT_TOP}, {MT_TOP_RIGHT}, %4, %5
   
   %3                                                       ; next line
   
   ; center
   mov                     xaxd, nHeightd
   sub                     xaxd, 2                          ; -2 : top & bottom lines
   
.loopy

   mov                     nHeightCountd, xaxd              ; save height counter

   MT_PROCESS_LINE         %1, %2, {MT_LEFT}, {MT_NORMAL}, {MT_RIGHT}, %4, %5

   %3                                                       ; next line
   
   mov                     xaxd, nHeightCountd              ; load height counter
   dec                     xaxd
   jnz                     .loopy
   
   ; bottom line
   
   MT_PROCESS_LINE         %1, %2, {MT_BOTTOM_LEFT}, {MT_BOTTOM}, {MT_BOTTOM_RIGHT}, %4, %5
%ENDMACRO