; -------------------------------------------------------------------------------
; This YASM header creates a set of defines and macros tools that can be useful
; for various stuff
; -------------------------------------------------------------------------------

; -------------------------------------------------------------------------------
; Summary of usable defines :
; Summary of usable macros :
;  - ASM_CONCATENATED_XDEFINE define_begin, define_end, definition
;  - ASM_XDEFINE_CONCATENATION define, definition_begin, definition_end
; -------------------------------------------------------------------------------

; -------------------------------------------------------------------------------
; ASM_CONCATENATED_XDEFINE a, b, c creates a define ab=c
; ASM_XDEFINE_CONCATENATION a, b, c creates a define a=bc
;
; Their usefulness lies in the possibility to expand a, b and c before creating
; the define. Furthermore, it avoids the sometimes unpredictible use of %+
; -------------------------------------------------------------------------------

%MACRO ASM_CONCATENATED_XDEFINE 3
   %XDEFINE %1%2 %3
%ENDMACRO

; -------------------------------------------------------------------------------

%MACRO ASM_XDEFINE_CONCATENATION 3-4
   %IF %0 = 4
      %XDEFINE %1 %2%3%4
   %ELSE
      %XDEFINE %1 %2%3
   %ENDIF
%ENDMACRO

; -------------------------------------------------------------------------------
; if cond, { instruction_cond_true } [[,else], { instruction_cond_false }]
; -------------------------------------------------------------------------------

%MACRO if 2-4
   %IF %1
      %2
   %ELIF %0 = 3
      %3
   %ELIF %0 = 4
      %4
   %ENDIF
%ENDMACRO

; -------------------------------------------------------------------------------
; ifidni sym1, sym2, { instruction_sym1=sym2 } [[,else], { instruction_sym1!=sym2 }]
; -------------------------------------------------------------------------------

%MACRO ifidni 3-5
   %IFIDNI %1, %2
      %3
   %ELIF %0 = 5
      %5
   %ELIF %0 = 4
      %4
   %ENDIF
%ENDMACRO

; -------------------------------------------------------------------------------

%MACRO ASM_NOP 0-*
%ENDMACRO