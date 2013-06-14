; -----------------------------------------------------------------------------
; Defines extension sets, cpus, and assign them numerical values
; -----------------------------------------------------------------------------

%ASSIGN _XARCH_EXTENSION_SET_NONE  0
%ASSIGN _XARCH_EXTENSION_SET_MMX   1
%ASSIGN _XARCH_EXTENSION_SET_ISSE  2
%ASSIGN _XARCH_EXTENSION_SET_SSE2  3
%ASSIGN _XARCH_EXTENSION_SET_SSE3  4
%ASSIGN _XARCH_EXTENSION_SET_SSSE3 5
%ASSIGN _XARCH_EXTENSION_SET_SSE41 6
%ASSIGN _XARCH_EXTENSION_SET_SSE42 7

; -----------------------------------------------------------------------------

%ASSIGN _XARCH_CPU_P1     0
%ASSIGN _XARCH_CPU_PMMX   1   
%ASSIGN _XARCH_CPU_PPRO   2
%ASSIGN _XARCH_CPU_P2     3
%ASSIGN _XARCH_CPU_P3     4
%ASSIGN _XARCH_CPU_PM     5
%ASSIGN _XARCH_CPU_K7     6
%ASSIGN _XARCH_CPU_K8     7
%ASSIGN _XARCH_CPU_P4     8
%ASSIGN _XARCH_CPU_P4E    9
%ASSIGN _XARCH_CPU_ATOM   10
%ASSIGN _XARCH_CPU_NANO   11
%ASSIGN _XARCH_CPU_CONROE 12
%ASSIGN _XARCH_CPU_K10    13
%ASSIGN _XARCH_CPU_PENRYN 14
%ASSIGN _XARCH_CPU_I7     15

; -----------------------------------------------------------------------------
; Some useful defines
; -----------------------------------------------------------------------------

%DEFINE HAS_MMX_REGISTERS ((_XARCH_EXTENSION_SET >= _XARCH_EXTENSION_SET_MMX && _XARCH_EXTENSION_SET <= _XARCH_EXTENSION_SET_ISSE) || HAS_FORCED_MMX == 1)
%DEFINE HAS_XMM_REGISTERS (_XARCH_EXTENSION_SET >= _XARCH_EXTENSION_SET_SSE2 && HAS_FORCED_MMX == 0)
%DEFINE HAS_SSE2          (_XARCH_EXTENSION_SET >= _XARCH_EXTENSION_SET_SSE2)
%DEFINE HAS_SSSE3         (_XARCH_EXTENSION_SET >= _XARCH_EXTENSION_SET_SSSE3)
%DEFINE HAS_SSE41         (_XARCH_EXTENSION_SET >= _XARCH_EXTENSION_SET_SSE41)
%DEFINE HAS_FAST_XMM      (_XARCH_CPU >= _XARCH_CPU_CONROE)

; -----------------------------------------------------------------------------

%MACRO SET_REGISTERS 0
   %IF HAS_MMX_REGISTERS
      %XDEFINE m0 mm0
      %XDEFINE m1 mm1
      %XDEFINE m2 mm2
      %XDEFINE m3 mm3
      %XDEFINE m4 mm4
      %XDEFINE m5 mm5
      %XDEFINE m6 mm6
      %XDEFINE m7 mm7
      %XDEFINE mshift 3
      %XDEFINE mwidth 8
      %XDEFINE mhwidth 4
   %ELIF HAS_XMM_REGISTERS
      %XDEFINE m0 xmm0
      %XDEFINE m1 xmm1
      %XDEFINE m2 xmm2
      %XDEFINE m3 xmm3
      %XDEFINE m4 xmm4
      %XDEFINE m5 xmm5
      %XDEFINE m6 xmm6
      %XDEFINE m7 xmm7
      %XDEFINE mshift 4
      %XDEFINE mwidth 16
      %XDEFINE mhwidth 8
   %ELSE
      %UNDEF m0
      %UNDEF m1
      %UNDEF m2
      %UNDEF m3
      %UNDEF m4
      %UNDEF m5
      %UNDEF m6
      %UNDEF m7
      %UNDEF mshift
      %UNDEF mwidth
      %UNDEF mhwidth
   %ENDIF
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO stohh_mmx 2
   punpckhdq %2, %2
   movd      %1, %2
%ENDMACRO   

; -----------------------------------------------------------------------------

%MACRO SET_INSTRUCTIONS 0
   ; TODO : use the float counterpart whenever possible. It generates smaller instructions
   ; thus reducing the strains on the instruction decoder. If doing so, take into account
   ; the fact that intel, in a very near future, plans to make the float unit and
   ; the integer unit completely distincts, inducing a penalty when going from one
   ; unit to another. In such a case, we might want to come back to the integer movs.
   %IF HAS_MMX_REGISTERS
      %XDEFINE mova  movq
      %XDEFINE movu  movq
      %XDEFINE movh  movd
      %XDEFINE movlh movd
      %XDEFINE movhh punpckldq
      %XDEFINE stohh stohh_mmx
   %ELIF HAS_XMM_REGISTERS
      %XDEFINE mova  movdqa
      %XDEFINE movu  movdqu
      %XDEFINE movh  movq
      %XDEFINE movlh movq
      %XDEFINE movhh movhpd
      %XDEFINE stohh movhpd
   %ELSE
      %ERROR _XARCH_EXTENSION_SET _XARCH_CPU
      %UNDEF mova
      %UNDEF movu
      %UNDEF movh
      %UNDEF movlh
      %UNDEF movhh
   %ENDIF
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO SET_XARCH 1-2
   %XDEFINE _XARCH_EXTENSION %1
   %IFIDNI %1, mmx
      %XDEFINE _XARCH_EXTENSION_SET   _XARCH_EXTENSION_SET_MMX
      %XDEFINE _XARCH_CPU             _XARCH_CPU_PMMX     
   %ELIFIDNI %1, isse   
      %XDEFINE _XARCH_EXTENSION_SET   _XARCH_EXTENSION_SET_ISSE
      %XDEFINE _XARCH_CPU             _XARCH_CPU_P3     
   %ELIFIDNI %1, sse2   
      %XDEFINE _XARCH_EXTENSION_SET   _XARCH_EXTENSION_SET_SSE2
      %XDEFINE _XARCH_CPU             _XARCH_CPU_P4
   %ELIFIDNI %1, sse3   
      %XDEFINE _XARCH_EXTENSION_SET   _XARCH_EXTENSION_SET_SSE3
      %XDEFINE _XARCH_CPU             _XARCH_CPU_P4
   %ELIFIDNI %1, ssse3   
      %XDEFINE _XARCH_EXTENSION_SET   _XARCH_EXTENSION_SET_SSSE3
      %XDEFINE _XARCH_CPU             _XARCH_CPU_CONROE
   %ELIFIDNI %1, sse41   
      %XDEFINE _XARCH_EXTENSION_SET   _XARCH_EXTENSION_SET_SSE41
      %XDEFINE _XARCH_CPU             _XARCH_CPU_PENRYN
   %ELIFIDNI %1, sse42  
      %XDEFINE _XARCH_EXTENSION_SET   _XARCH_EXTENSION_SET_SSE42
      %XDEFINE _XARCH_CPU             _XARCH_CPU_I7
   %ELIFIDNI %1, p1
      %XDEFINE _XARCH_EXTENSION_SET   _XARCH_EXTENSION_SET_NONE
      %XDEFINE _XARCH_CPU             _XARCH_CPU_P1
   %ELIFIDNI %1, pmmx
      %XDEFINE _XARCH_EXTENSION_SET   _XARCH_EXTENSION_SET_MMX
      %XDEFINE _XARCH_CPU             _XARCH_CPU_PMMX
   %ELIFIDNI %1, ppro
      %XDEFINE _XARCH_EXTENSION_SET   _XARCH_EXTENSION_SET_NONE
      %XDEFINE _XARCH_CPU             _XARCH_CPU_PPRO
   %ELIFIDNI %1, p2
      %XDEFINE _XARCH_EXTENSION_SET   _XARCH_EXTENSION_SET_ISSE
      %XDEFINE _XARCH_CPU             _XARCH_CPU_P3
   %ELIFIDNI %1, p3
      %XDEFINE _XARCH_EXTENSION_SET   _XARCH_EXTENSION_SET_ISSE
      %XDEFINE _XARCH_CPU             _XARCH_CPU_P3
   %ELIFIDNI %1, pm
   ; Technically, Pentium M supports SSE2. However, it is said to be so slow as to be useless, so we will use ISSE instead
      %XDEFINE _XARCH_EXTENSION_SET   _XARCH_EXTENSION_SET_ISSE
      %XDEFINE _XARCH_CPU             _XARCH_CPU_PM
   %ELIFIDNI %1, p4
      %XDEFINE _XARCH_EXTENSION_SET   _XARCH_EXTENSION_SET_SSE2
      %XDEFINE _XARCH_CPU             _XARCH_CPU_P4
   %ELIFIDNI %1, p4e
      %XDEFINE _XARCH_EXTENSION_SET   _XARCH_EXTENSION_SET_SSE3
      %XDEFINE _XARCH_CPU             _XARCH_CPU_P4E
   %ELIFIDNI %1, atom
      %XDEFINE _XARCH_EXTENSION_SET   _XARCH_EXTENSION_SET_SSSE3
      %XDEFINE _XARCH_CPU             _XARCH_CPU_ATOM
   %ELIFIDNI %1, nano
      %XDEFINE _XARCH_EXTENSION_SET   _XARCH_EXTENSION_SET_SSSE3
      %XDEFINE _XARCH_CPU             _XARCH_CPU_NANO
   %ELIFIDNI %1, conroe
      %XDEFINE _XARCH_EXTENSION_SET   _XARCH_EXTENSION_SET_SSSE3
      %XDEFINE _XARCH_CPU             _XARCH_CPU_CONROE
   %ELIFIDNI %1, penryn
      %XDEFINE _XARCH_EXTENSION_SET   _XARCH_EXTENSION_SET_SSE41
      %XDEFINE _XARCH_CPU             _XARCH_CPU_PENRYN
   %ELIFIDNI %1, i7
      %XDEFINE _XARCH_EXTENSION_SET   _XARCH_EXTENSION_SET_SSE42
      %XDEFINE _XARCH_CPU             _XARCH_CPU_I7
   %ELIFIDNI %1, k7
      %XDEFINE _XARCH_EXTENSION_SET   _XARCH_EXTENSION_SET_ISSE
      %XDEFINE _XARCH_CPU             _XARCH_CPU_K7
   %ELIFIDNI %1, k8
   ; Technically, K8 supports SSE2. But I never managed to be faster than ISSE with it, though this time, it might be my doing
      %XDEFINE _XARCH_EXTENSION_SET   _XARCH_EXTENSION_SET_ISSE
      %XDEFINE _XARCH_CPU             _XARCH_CPU_K8
   %ELIFIDNI %1, k10
   ; We will have to add SSE4A... when we find a need for it
      %XDEFINE _XARCH_EXTENSION_SET   _XARCH_EXTENSION_SET_SSE2
      %XDEFINE _XARCH_CPU             _XARCH_CPU_K10
   %ELSE
      %ERROR Invalid extension for a function. Only p1, pmmx, ppro, p2, p3, pm, p4, atom, nano, conroe, penryn, i7, k7, k8 and k10 are allowed
   %ENDIF
   
   %IF %0 = 2
      %IFIDNI %2, mmx
         %IF _XARCH_EXTENSION_SET >= _XARCH_EXTENSION_SET_SSE41
            %ERROR SSE41 and SSE42 instruction sets do not support MMX registers
         %ENDIF
         %DEFINE HAS_FORCED_MMX 1
      %ELSE
         %DEFINE HAS_FORCED_MMX 0
      %ENDIF
   %ELSE
      %DEFINE HAS_FORCED_MMX 0
   %ENDIF
   SET_REGISTERS
   SET_INSTRUCTIONS
%ENDMACRO

; -----------------------------------------------------------------------------

; Copy-paste lowest byte of each dword into all of the dword bytes
%MACRO splatbd 1
   %IF HAS_SSSE3
      pshufb         %1, [ATUTILS_SPLAT_BYTE_DWORD]
   %ELIF HAS_XMM_REGISTERS
      pmaddwd        %1, [ATUTILS_BYTE_1]
      pshuflw        %1, %1, 0xA0
      pshufhw        %1, %1, 0xA0
   %ELIF HAS_ISSE
      pmaddwd        %1, [ATUTILS_BYTE_1]
      pshufw         %1, %1, 0xA0
   %ELSE
      %ERROR You need at least ISSE extension set for splatbd
   %ENDIF      
%ENDMACRO

; -----------------------------------------------------------------------------

; Copy-paste lowest byte of the register into all of its bytes
%MACRO splatbr 1
   %IF HAS_SSSE3
      pshufb         %1, [ATUTILS_SPLAT_BYTE_DQWORD]
   %ELIF HAS_XMM_REGISTERS
      ; TODO : can be improved, using pshuflw/pshufhw
      punpcklbw      %1, %1
      punpcklbw      %1, %1
      punpcklbw      %1, %1
      punpcklbw      %1, %1
   %ELIF HAS_ISSE
      punpcklbw      %1, %1
      pshufw         %1, %1, 00000000b
   %ELSE
      %ERROR You need at least ISSE extension set for splatbr
   %ENDIF      
%ENDMACRO

; -----------------------------------------------------------------------------

; Copy-paste lowest word of the register into all of its words
%MACRO splatwr 1
   %IF HAS_SSSE3
      pshufb         %1, [ATUTILS_SPLAT_WORD_DQWORD]
   %ELIF HAS_XMM_REGISTERS
      ; TODO : might be improved, using pshuflw/pshufhw
      punpcklwd      %1, %1
      punpcklwd      %1, %1
      punpcklwd      %1, %1
   %ELIF HAS_ISSE
      pshufw         %1, %1, 00000000b
   %ELSE
      %ERROR You need at least ISSE extension set for splatwr
   %ENDIF      
%ENDMACRO

; -----------------------------------------------------------------------------

; Horizontal sum of all four quarters of the register (thus all dwords for xmm, all
; words for mmx), result stored in the lowest dword (whatever the register size).
; Other dwords are filled with garbage. Second parameter is a temporary.
%MACRO phsumqrd 2
   %IF HAS_SSSE3
      phaddd         %1, %1
      phaddd         %1, %1
   %ELIF HAS_SSE2
      pshufd         %2, %1, 0x4E
      paddd          %1, %2
      pshufd         %2, %1, 0x11
      paddd          %1, %2
   %ELIF HAS_ISSE
      pmaddwd       %1, [ATUTILS_WORD_1]
      pshufw        %2, %1, 0xC6
      paddw         %1, %2
   %ELSE
      %ERROR You need at least ISSE extension set for phsumqr
   %ENDIF
%ENDMACRO   

; -----------------------------------------------------------------------------

; Two possible syntaxes :
;  - movaoff dest, address, offset
;  - movaoff dest, temp, address, offset
; It does movu dest, [address + offset] and requires address to be reg-width bytes aligned,
; The first syntax requires SSSE3. The second one will also work with MMX/SSE2, but isn't
; yet implemented
%MACRO movaoff 3-4
   %DEFINE %%DESTINATION %1
   %IF %0 > 3
      %DEFINE %%SPARE %2
      %ROTATE 1
   %ENDIF
   %IF HAS_SSSE3
      mova     %%DESTINATION, [%2 + mwidth]
      palignr  %%DESTINATION, [%2], %3
   %ELSE
      %IF %0 = 3
         %ERROR movaoff requires at least SSSE3, or a spare register
      %ELSE
         %ERROR TODO : code with movaoff without palignr, but with psrlq/psrldq instead
      %ENDIF 
   %ENDIF
%ENDMACRO

; Two syntaxes :
;  - movaoffx2 dest0, dest1, address0, address1, offset
;  - movaoffx2 dest0, dest1, temp0, temp1, address0, address1, offset
%MACRO movaoffx2 5-7
   %DEFINE %%DESTINATION_0 %1
   %DEFINE %%DESTINATION_1 %2
   %IF %0 > 5
      %DEFINE %%SPARE_0 %3
      %DEFINE %%SPARE_1 %4
      %ROTATE 2
   %ENDIF
   %IF HAS_SSSE3
      mova     %%DESTINATION_0, [%3 + mwidth]
      mova     %%DESTINATION_1, [%4 + mwidth]
      palignr  %%DESTINATION_0, [%3], %5
      palignr  %%DESTINATION_1, [%4], %5
   %ELSE
      %IF %0 = 5
         %ERROR movaoffx2 requires at least SSSE3, or two spare registers
      %ELSE
         %ERROR TODO : code with movaoffx2 without palignr, but with psrlq/psrldq instead
      %ENDIF 
   %ENDIF
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO doif 2-4
   %IF %1
      %2
   %ELIF %0 > 3
      %4
   %ELIF %0 > 2
      %3
   %ENDIF
%ENDMACRO   

; -----------------------------------------------------------------------------

%MACRO doifidni 3-5
   %IFIDNI %1, %2
      %3
   %ELIF %0 > 4
      %5
   %ELIF %0 > 3
      %4
   %ENDIF
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO doifnidni 3-5
   %IFNIDNI %1, %2
      %3
   %ELIF %0 > 4
      %5
   %ELIF %0 > 3
      %4
   %ENDIF
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO doifeq 3
   doifidni %2, %3, {%1 %2, %3}
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO doifneq 3
   doifnidni %2, %3, {%1 %2, %3}
%ENDMACRO   

; -----------------------------------------------------------------------------

; Creates a jump table. Syntax is :
;
;    jmptable reg, first_index, last_index, macro, macro_params
; 
; What it does is instanciate the macro several times, each times with a 
; different label. Then according to the value stored in reg, the address of
; the corresponding instance of the macro will be computed and jumped to.
;
; macro will take at least two parameters, the first one being the base name
; of the jump label, the second one being the index of the instance. 
; .base_name %+ index is the address of the instance of the macro
; .base_name %+ end is the address the macro must jump to when it needs early exit
; if macro needs additional parameters, they can be given through macro_params (with 
; the syntax { x, y, z} for example)

%MACRO jmptable 5
   %ASSIGN %%FIRST_INDEX %2
   %ASSIGN %%LAST_INDEX %3
   %ASSIGN %%SECOND_INDEX %%FIRST_INDEX + 1

   doif %%FIRST_INDEX > 0, {sub %1, %%FIRST_INDEX}
   imul %1, .%%label %+ %%SECOND_INDEX - .%%label %+ %%FIRST_INDEX
   add %1, .%%label %+ %%FIRST_INDEX
   jmp %1
   
   %ASSIGN %%INDEX %%FIRST_INDEX
   %REP %%LAST_INDEX - %%FIRST_INDEX + 1
      ALIGN 16
      .%%label %+ %%INDEX
      %4 %%label, %%INDEX, %5
      %ASSIGN %%INDEX %%INDEX + 1
      jmp .%%label %+ end
   %ENDREP

   .%%label %+ end
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO quick_alignment_check 3
   %IF %2 = 8
      %ASSIGN %%MASK 0x3F
      %ASSIGN %%COMP 0x38
   %ELIF %2 = 16
      %ASSIGN %%MASK 0x37
      %ASSIGN %%COMP 0x30
   %ELIF %2 = 4
      %ASSIGN %%MASK 0x3F
      %ASSIGN %%COMP 0x3C
   %ELSE
      %ERROR Unsupported width for alignment checks. Only 4, 8 or 16 are supported
   %ENDIF
   ARGUMENTS %1, %%NAME
   and %%NAME, %%MASK
   cmp %%NAME, %%COMP
   ENDARGUMENTS
   jle %3
%ENDMACRO
   
; -----------------------------------------------------------------------------

; Takes two simd register indexes, and swap the defines of the registers. 
; For example, in mmx, after swapreg 0, 1, m0 will be exanded into mm1, and m1 into
; mm0.

%MACRO swapreg 2
   %XDEFINE __t__ m%1
   %XDEFINE m%1 m%2
   %XDEFINE m%2 __t__
   %UNDEF __t__
%ENDMACRO

; -----------------------------------------------------------------------------
