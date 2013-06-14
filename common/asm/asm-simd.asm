; -------------------------------------------------------------------------------
; This YASM header creates a set of defines and macros that are useful
; when it comes to simd. The purpose is to reduce as much as possible the
; need for code duplication when writing the same function for different simd
; architecture.
; -------------------------------------------------------------------------------

; -------------------------------------------------------------------------------
; Summary of defines :
;  - ASM_SIMD is a set of flags containing the supported simd instruction sets
;  - ASM_CPU is the CPU the function is targetting
;  - ASM_EXTENSION is the extension given to ASM_XMMX
;  - ASM_HAS_SSE2, ASM_HAS_SSSE3, ASM_HAS_SSE41 indicates if those instruction sets
; are supported
;  - ASM_HAS_MMX_REGISTERS, ASM_HAS_XMM_REGISTERS indicates to which simd registers
; (mmX, xmmX) mX registers are aliased to
;  - ASM_HAS_FORCED_MMX indicates whether mmx registers were forced (that happens
; when you're planning to use SSE2+ extension sets in conjonction with mmx registers)
; Summary of macros :
;  - ASM_XMMX extension [, mmx] : initialize the various defines to their proper values,
; and create the simdless registers and instructions.
; Summary of pseudo instructions :
;  - mova   a, b : aligned move of b into a
;  - movu   a, b : unaligned move of b into a
;  - movh   a, b : move b into the low part of a. The high part of a is destroyed
;  - psplatbd a  : copy the first byte of each dword of a into all the bytes of the dword
;  - psplatbr a  : copy the first byte of a into the all its bytes
;  - phsumqrd a, tmp : add the four quarter of the register into the low dword
;  - pswapm n, m : swap mN and mM defines
; -------------------------------------------------------------------------------

%ASSIGN ASM_SIMD_NONE   0x000
%ASSIGN ASM_SIMD_MMX    0x001
%ASSIGN ASM_SIMD_3DNOW  0x002
%ASSIGN ASM_SIMD_3DNOW2 0x004
%ASSIGN ASM_SIMD_ISSE   0x008
%ASSIGN ASM_SIMD_SSE2   0x010
%ASSIGN ASM_SIMD_SSE3   0x020
%ASSIGN ASM_SIMD_SSSE3  0x040
%ASSIGN ASM_SIMD_SSE41  0x080
%ASSIGN ASM_SIMD_SSE42  0x100
%ASSIGN ASM_SIMD_SSE4A  0x200

; -------------------------------------------------------------------------------

%ASSIGN ASM_SIMD_UP_TO_MMX      ASM_SIMD_MMX
%ASSIGN ASM_SIMD_UP_TO_ISSE     ASM_SIMD_ISSE  | ASM_SIMD_UP_TO_MMX
%ASSIGN ASM_SIMD_UP_TO_3DNOW    ASM_SIMD_3DNOW | ASM_SIMD_UP_TO_ISSE
%ASSIGN ASM_SIMD_UP_TO_3DNOW2   ASM_SIMD_3DNOW2 | ASM_SIMD_UP_TO_3DNOW
%ASSIGN ASM_SIMD_UP_TO_SSE2     ASM_SIMD_SSE2  | ASM_SIMD_UP_TO_ISSE
%ASSIGN ASM_SIMD_UP_TO_SSE3     ASM_SIMD_SSE3  | ASM_SIMD_UP_TO_SSE2
%ASSIGN ASM_SIMD_UP_TO_SSSE3    ASM_SIMD_SSSE3 | ASM_SIMD_UP_TO_SSE3
%ASSIGN ASM_SIMD_UP_TO_SSE41    ASM_SIMD_SSE41 | ASM_SIMD_UP_TO_SSSE3
%ASSIGN ASM_SIMD_UP_TO_SSE42    ASM_SIMD_SSE42 | ASM_SIMD_UP_TO_SSE41
%ASSIGN ASM_SIMD_UP_TO_SSE4A    ASM_SIMD_SSE4A | ASM_SIMD_UP_TO_3DNOW2 | ASM_SIMD_UP_TO_SSE3

; -------------------------------------------------------------------------------

%ASSIGN ASM_CPU_P1      0
%ASSIGN ASM_CPU_PMMX    1   
%ASSIGN ASM_CPU_PPRO    2
%ASSIGN ASM_CPU_P2      3
%ASSIGN ASM_CPU_P3      4
%ASSIGN ASM_CPU_PM      5
%ASSIGN ASM_CPU_K7      6
%ASSIGN ASM_CPU_K8      7
%ASSIGN ASM_CPU_P4      8
%ASSIGN ASM_CPU_P4E     9
%ASSIGN ASM_CPU_ATOM    10
%ASSIGN ASM_CPU_NANO    11
%ASSIGN ASM_CPU_CONROE  12
%ASSIGN ASM_CPU_K10     13
%ASSIGN ASM_CPU_PENRYN  14
%ASSIGN ASM_CPU_I7      15

; -------------------------------------------------------------------------------

%DEFINE ASM_HAS_SSE2          (ASM_SIMD &  ASM_SIMD_SSE2 == ASM_SIMD_SSE2)
%DEFINE ASM_HAS_SSSE3         (ASM_SIMD & ASM_SIMD_SSSE3 == ASM_SIMD_SSSE3)
%DEFINE ASM_HAS_SSE41         ((ASM_SIMD & ASM_SIMD_SSE41 == ASM_SIMD_SSE41) && ASM_HAS_FORCED_MMX == 0)
%DEFINE ASM_HAS_MMX_REGISTERS (ASM_HAS_SSE2 == 0 || ASM_HAS_FORCED_MMX)
%DEFINE ASM_HAS_XMM_REGISTERS (ASM_HAS_SSE2 && ASM_HAS_FORCED_MMX == 0)
%DEFINE ASM_HAS_FAST_XMM      (ASM_CPU >= ASM_CPU_CONROE)
%DEFINE ASM_HAS_PREFETCHTX    (ASM_SIMD & ASM_SIMD_ISSE)
%DEFINE ASM_HAS_PREFETCHW     (ASM_SIMD & ASM_SIMD_3DNOW)

; -------------------------------------------------------------------------------

%MACRO ASM_SET_REGISTERS 0
   %IF ASM_HAS_MMX_REGISTERS
      %XDEFINE m0 mm0
      %XDEFINE m1 mm1
      %XDEFINE m2 mm2
      %XDEFINE m3 mm3
      %XDEFINE m4 mm4
      %XDEFINE m5 mm5
      %XDEFINE m6 mm6
      %XDEFINE m7 mm7
      %ASSIGN mshift 3
   %ELIF ASM_HAS_XMM_REGISTERS
      %XDEFINE m0 xmm0
      %XDEFINE m1 xmm1
      %XDEFINE m2 xmm2
      %XDEFINE m3 xmm3
      %XDEFINE m4 xmm4
      %XDEFINE m5 xmm5
      %XDEFINE m6 xmm6
      %XDEFINE m7 xmm7
      %ASSIGN mshift 4
   %ELSE
      %UNDEF m0
      %UNDEF m1
      %UNDEF m2
      %UNDEF m3
      %UNDEF m4
      %UNDEF m5
      %UNDEF m6
      %UNDEF m7
      %ASSIGN mshift 0
   %ENDIF
   %ASSIGN mfull 1 << mshift
   %ASSIGN mhalf 1 << (mshift - 1)
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO mova 2
   %IF ASM_HAS_MMX_REGISTERS
      movq %1, %2
   %ELSE
      movdqa %1, %2
   %ENDIF
%ENDMACRO   

; -----------------------------------------------------------------------------

%MACRO movu 2
   %IF ASM_HAS_MMX_REGISTERS
      movq %1, %2
   %ELSE
      movdqu %1, %2
   %ENDIF
%ENDMACRO   

; -----------------------------------------------------------------------------

%MACRO movau 2
   %IF ASM_HAS_ALIGNED_LOAD
      mova %1, %2
   %ELSE
      movu %1, %2
   %ENDIF
%ENDMACRO   

%MACRO ASM_SET_INSTRUCTIONS 0
   %IF ASM_HAS_MMX_REGISTERS
      %XDEFINE movh  movd
   %ELIF ASM_HAS_XMM_REGISTERS
      %XDEFINE movh  movq
   %ELSE
      %UNDEF movh
   %ENDIF
   
   %XDEFINE prefetchr   ASM_NOP
   %XDEFINE prefetchrw  ASM_NOP
   %XDEFINE prefetchnta ASM_NOP
   %IF ASM_HAS_PREFETCHTX
      %XDEFINE prefetchr   prefetcht0
      %XDEFINE prefetchrw  prefetcht0
      %XDEFINE prefetchnta prefetchnta
   %ENDIF
   
   %IF ASM_HAS_PREFETCHW
      %XDEFINE prefetchrw  prefetchw
   %ENDIF
   
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO ASM_XMMX 1-*
   %XDEFINE ASM_EXTENSION %1
   %DEFINE  ASM_HAS_ALIGNED_LOAD (0)
   %DEFINE  ASM_HAS_FORCED_MMX   (0)
   
   %IFIDNI %1, mmx
      %XDEFINE ASM_SIMD   ASM_SIMD_UP_TO_MMX
      %XDEFINE ASM_CPU    ASM_CPU_PMMX     
   %ELIFIDNI %1, isse   
      %XDEFINE ASM_SIMD   ASM_SIMD_UP_TO_ISSE
      %XDEFINE ASM_CPU    ASM_CPU_P3     
   %ELIFIDNI %1, 3dnow
      %XDEFINE ASM_SIMD   ASM_SIMD_UP_TO_3DNOW
      %XDEFINE ASM_CPU    ASM_CPU_K7
   %ELIFIDNI %1, 3dnow2
      %XDEFINE ASM_SIMD   ASM_SIMD_UP_TO_3DNOW2
      %XDEFINE ASM_CPU    ASM_CPU_K7
   %ELIFIDNI %1, sse2   
      %XDEFINE ASM_SIMD   ASM_SIMD_UP_TO_SSE2
      %XDEFINE ASM_CPU    ASM_CPU_P4
   %ELIFIDNI %1, asse2   
      %XDEFINE ASM_SIMD   ASM_SIMD_UP_TO_SSE2
      %XDEFINE ASM_CPU    ASM_CPU_P4
      %DEFINE  ASM_HAS_ALIGNED_LOAD (1)
   %ELIFIDNI %1, sse3   
      %XDEFINE ASM_SIMD   ASM_SIMD_UP_TO_SSE3
      %XDEFINE ASM_CPU    ASM_CPU_P4
   %ELIFIDNI %1, asse3   
      %XDEFINE ASM_SIMD   ASM_SIMD_UP_TO_SSE3
      %XDEFINE ASM_CPU    ASM_CPU_P4
      %DEFINE  ASM_HAS_ALIGNED_LOAD (1)
   %ELIFIDNI %1, ssse3   
      %XDEFINE ASM_SIMD   ASM_SIMD_UP_TO_SSSE3
      %XDEFINE ASM_CPU    ASM_CPU_CONROE
   %ELIFIDNI %1, assse3   
      %XDEFINE ASM_SIMD   ASM_SIMD_UP_TO_SSSE3
      %XDEFINE ASM_CPU    ASM_CPU_CONROE
      %DEFINE  ASM_HAS_ALIGNED_LOAD (1)
   %ELIFIDNI %1, sse41   
      %XDEFINE ASM_SIMD   ASM_SIMD_UP_TO_SSE41
      %XDEFINE ASM_CPU    ASM_CPU_PENRYN
   %ELIFIDNI %1, asse41   
      %XDEFINE ASM_SIMD   ASM_SIMD_UP_TO_SSE41
      %XDEFINE ASM_CPU    ASM_CPU_PENRYN
      %DEFINE  ASM_HAS_ALIGNED_LOAD (1)
   %ELIFIDNI %1, sse42  
      %XDEFINE ASM_SIMD   ASM_SIMD_UP_TO_SSE42
      %XDEFINE ASM_CPU    ASM_CPU_I7
   %ELIFIDNI %1, asse42  
      %XDEFINE ASM_SIMD   ASM_SIMD_UP_TO_SSE42
      %XDEFINE ASM_CPU    ASM_CPU_I7
      %DEFINE  ASM_HAS_ALIGNED_LOAD (1)
   %ELIFIDNI %1, sse4a
      %XDEFINE ASM_SIMD   ASM_SIMD_UP_TO_SSE4A
      %XDEFINE ASM_CPU    ASM_CPU_K10
   %ELIFIDNI %1, asse4a
      %XDEFINE ASM_SIMD   ASM_SIMD_UP_TO_SSE4A
      %XDEFINE ASM_CPU    ASM_CPU_K10
      %DEFINE  ASM_HAS_ALIGNED_LOAD (1)
   %ELIFIDNI %1, p1
      %XDEFINE ASM_SIMD   ASM_SIMD_NONE
      %XDEFINE ASM_CPU    ASM_CPU_P1
   %ELIFIDNI %1, pmmx
      %XDEFINE ASM_SIMD   ASM_SIMD_UP_TO_MMX
      %XDEFINE ASM_CPU    ASM_CPU_PMMX
   %ELIFIDNI %1, ppro
      %XDEFINE ASM_SIMD   ASM_SIMD_UP_TO_NONE
      %XDEFINE ASM_CPU    ASM_CPU_PPRO
   %ELIFIDNI %1, p2
      %XDEFINE ASM_SIMD   ASM_SIMD_UP_TO_ISSE
      %XDEFINE ASM_CPU    ASM_CPU_P3
   %ELIFIDNI %1, p3
      %XDEFINE ASM_SIMD   ASM_SIMD_UP_TO_ISSE
      %XDEFINE ASM_CPU    ASM_CPU_P3
   %ELIFIDNI %1, pm
   ; Technically, Pentium M supports SSE2. However, it is said to be so slow as to be useless, so we will use ISSE instead
      %XDEFINE ASM_SIMD   ASM_SIMD_UP_TO_ISSE
      %XDEFINE ASM_CPU    ASM_CPU_PM
   %ELIFIDNI %1, p4
      %XDEFINE ASM_SIMD   ASM_SIMD_UP_TO_SSE2
      %XDEFINE ASM_CPU    ASM_CPU_P4
   %ELIFIDNI %1, p4e
      %XDEFINE ASM_SIMD   ASM_SIMD_UP_TO_SSE3
      %XDEFINE ASM_CPU    ASM_CPU_P4E
   %ELIFIDNI %1, atom
      %XDEFINE ASM_SIMD   ASM_SIMD_UP_TO_SSSE3
      %XDEFINE ASM_CPU    ASM_CPU_ATOM
   %ELIFIDNI %1, nano
      %XDEFINE ASM_SIMD   ASM_SIMD_UP_TO_SSSE3
      %XDEFINE ASM_CPU    ASM_CPU_NANO
   %ELIFIDNI %1, conroe
      %XDEFINE ASM_SIMD   ASM_SIMD_UP_TO_SSSE3
      %XDEFINE ASM_CPU    ASM_CPU_CONROE
   %ELIFIDNI %1, penryn
      %XDEFINE ASM_SIMD   ASM_SIMD_UP_TO_SSE41
      %XDEFINE ASM_CPU    ASM_CPU_PENRYN
   %ELIFIDNI %1, i7
      %XDEFINE ASM_SIMD   ASM_SIMD_UP_TO_SSE42
      %XDEFINE ASM_CPU    ASM_CPU_I7
   %ELIFIDNI %1, k7
      %XDEFINE ASM_SIMD   ASM_SIMD_UP_TO_ISSE
      %XDEFINE ASM_CPU    ASM_CPU_K7
   %ELIFIDNI %1, k8
   ; Technically, K8 supports SSE2. But I never managed to be faster than ISSE with it, though this time, it might be my doing
      %XDEFINE ASM_SIMD   ASM_SIMD_UP_TO_ISSE
      %XDEFINE ASM_CPU    ASM_CPU_K8
   %ELIFIDNI %1, k10
   ; We will have to add SSE4A... when we find a need for it
      %XDEFINE ASM_SIMD   ASM_SIMD_UP_TO_SSE2
      %XDEFINE ASM_CPU    ASM_CPU_K10
   %ELSE
      %ERROR Invalid extension for a function. Only mmx, isse, 3dnow, 3dnow2, sse2, sse3, ssse3, sse41, sse42, sse4a, asse2, asse3, assse3, asse41, asse42, asse4a, p1, pmmx, ppro, p2, p3, pm, p4, atom, nano, conroe, penryn, i7, k7, k8 and k10 are allowed
   %ENDIF
   
   %REP %0 - 1
      %IFIDNI %2, mmx
         %DEFINE ASM_HAS_FORCED_MMX (1)
      %ELIFIDNI %2, aligned
         %DEFINE ASM_HAS_ALIGNED_LOAD (1)
      %ELSE
         %ERROR Additional arguments for ASM_XMMX must be either mmx or aligned
      %ENDIF
      %ROTATE 1
   %ENDREP
   
   ASM_SET_REGISTERS
   ASM_SET_INSTRUCTIONS
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO ASM_START_MMX 0
   %IF ASM_HAS_XMM_REGISTERS
      %DEFINE ASM_HAS_FORCED_MMX (1)
   %ENDIF
   
   ASM_SET_REGISTERS
   ASM_SET_INSTRUCTIONS
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO ASM_STOP_MMX 0
   %DEFINE ASM_HAS_FORCED_MMX (0)
   
   ASM_SET_REGISTERS
   ASM_SET_INSTRUCTIONS
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO psplatbd 1
   %IF ASM_HAS_SSSE3
      pshufb         %1, [ASM_SPLAT_BYTE_DWORD]
   %ELIF ASM_HAS_XMM_REGISTERS
      pmaddwd        %1, [ASM_BYTE_1]
      pshuflw        %1, %1, 10100000b
      pshufhw        %1, %1, 10100000b
   %ELIF ASM_HAS_ISSE
      pmaddwd        %1, [ASM_BYTE_1]
      pshufw         %1, %1, 10100000b
   %ELSE
      %ERROR You need at least ISSE extension set for psplatbd
   %ENDIF      
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO psplatbr 1
   %IF ASM_HAS_SSSE3
      pshufb         %1, [ATUTILS_SPLAT_BYTE_DQWORD]
   %ELIF ASM_HAS_XMM_REGISTERS
      ; TODO : can be improved, using pshuflw/pshufhw
      punpcklbw      %1, %1
      punpcklbw      %1, %1
      punpcklbw      %1, %1
      punpcklbw      %1, %1
   %ELIF ASM_HAS_ISSE
      punpcklbw      %1, %1
      pshufw         %1, %1, 00000000b
   %ELSE
      %ERROR You need at least ISSE extension set for psplatbr
   %ENDIF      
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO psplatwr 1
   %IF ASM_HAS_SSSE3
      pshufb         %1, [ATUTILS_SPLAT_WORD_DQWORD]
   %ELIF ASM_HAS_XMM_REGISTERS
      ; TODO : can be improved, using pshuflw/pshufhw
      punpcklwd      %1, %1
      punpcklwd      %1, %1
      punpcklwd      %1, %1
   %ELIF ASM_HAS_ISSE
      pshufw         %1, %1, 00000000b
   %ELSE
      %ERROR You need at least ISSE extension set for psplatbr
   %ENDIF      
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO phsumqrd 2
   %IF ASM_HAS_SSSE3
      phaddd         %1, %1
      phaddd         %1, %1
   %ELIF ASM_HAS_SSE2
      pshufd         %2, %1, 01001110b
      paddd          %1, %2
      pshufd         %2, %1, 00010001b
      paddd          %1, %2
   %ELIF ASM_HAS_ISSE
      pmaddwd       %1, [ASM_WORD_1]
      pshufw        %2, %1, 11000110b
      paddw         %1, %2
   %ELSE
      %ERROR You need at least ISSE extension set for phsumqr
   %ENDIF
%ENDMACRO   

; -----------------------------------------------------------------------------

%MACRO pswapm 2
   %XDEFINE %%TEMP m%1
   %XDEFINE m%1 m%2
   %XDEFINE m%2 %%TEMP
   %UNDEF %%TEMP
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO pshift 2
   %IF ASM_HAS_MMX_REGISTERS
      %IF %2 >= 0
         psllq %1, (%2) * 8
      %ELSE
         psrlq %1, (%2) * -8
      %ENDIF
   %ELSE
      %IF %2 >= 0
         pslldq %1, %2
      %ELSE
         psrldq %1, -(%2)
      %ENDIF
   %ENDIF
%ENDMACRO   