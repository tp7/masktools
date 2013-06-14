; -------------------------------------------------------------------------------
; All internal defines and macros are capsized and prefixed by ASM_*,
; External defines and macros aren't prefixed, in order not to make them
; abusively long.
;
; You are strongly advised not to use any defines/macros prefixed by ASM_
; to avoid conflicting with this header.
; -------------------------------------------------------------------------------

%INCLUDE "asm-tools.asm"
%INCLUDE "asm-simd.asm"
%INCLUDE "asm-portability.asm"
%INCLUDE "asm-constants.asm"