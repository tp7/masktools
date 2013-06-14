; -------------------------------------------------------------------------------
; This YASM header creates a set of defines and macros that can be used
; to create win32, lin32, win64 and lin64 compatible assembly functions
; -------------------------------------------------------------------------------

; -------------------------------------------------------------------------------
; Summary of usable defines :
;  - ASM_OS_WIN or ASM_OS_LIN
;  - ASM_OS which is either "lin" or "win"
;  - ASM_CHIP_X86 or ASM_CHIP_X64
;  - ASM_CHIP which is either "x64" or "x86"
;  - ASM_BITS which is either "32" or "64"
;  - ASM_BYTES which is either "4" or "8"
;  - ASM_UNDERSCORE or ASM_NO_UNDERSCORE
;  - ASM_MANGLING which is either "" or "_"
; Summary of usable macros :
;  - ASM_GLOBAL function_name : creates a define "function_name" with the proper 
; name mangling. Also declare the function.
;  - ASM_EXTERN function_name : creates a define "function_name" with the proper 
; name mangling. Also declare that external function.
;  - ASM_SECTION_RODATA which declares a read-only data section
; -------------------------------------------------------------------------------




; -------------------------------------------------------------------------------
;  Architecture definition :
;    - only one of ASM_CHIP_X86 and ASM_CHIP_X64 can be defined
;    - only one of ASM_OS_LIN and ASM_OS_WIN can be defined
;    - ASM_CHIP will expand either to x86 or to x64.
;    - ASM_OS will expand to either win or lin
;    - ASM_BITS will expand to either 32 or 64
;    - architecture is inferred from output format, but can be forced through
;  external defines : either use -DASM_CHIP_X86 or -DASM_CHIP_X64, and -DASM_OS_WIN or
;  -DASM_OS_LIN
; -------------------------------------------------------------------------------

%IFNDEF ASM_OS_LIN
   %IFNDEF ASM_OS_WIN
      %IFIDN __OUTPUT_FORMAT__, elf64
         %DEFINE ASM_OS_LIN
      %ELIFIDN __OUTPUT_FORMAT__, elf32
         %DEFINE ASM_OS_LIN
      %ELIFIDN __OUTPUT_FORMAT__, win32
         %DEFINE ASM_OS_WIN
      %ELIFIDN __OUTPUT_FORMAT__, win64
         %DEFINE ASM_OS_WIN
      %ELSE
         %ERROR Neither ASM_OS_WIN nor ASM_OS_LIN are defined, and the target operating system can't be inferred from the output format
      %ENDIF
   %ENDIF
%ENDIF

; -------------------------------------------------------------------------------

%IFDEF ASM_OS_WIN
   %IFDEF ASM_OS_LIN
      %ERROR Only one of ASM_OS_WIN and ASM_OS_LIN can be defined at the same time
   %ENDIF
%ENDIF

; -------------------------------------------------------------------------------

%IFDEF ASM_OS_WIN
   %DEFINE ASM_OS win
%ELSE
   %DEFINE ASM_OS lin
%ENDIF      

; -------------------------------------------------------------------------------

%IFNDEF ASM_CHIP_X86
   %IFNDEF ASM_CHIP_X64
      %IFIDN __OUTPUT_FORMAT__, elf64
         %DEFINE ASM_CHIP_X64
      %ELIFIDN __OUTPUT_FORMAT__, elf32
         %DEFINE ASM_CHIP_X86
      %ELIFIDN __OUTPUT_FORMAT__, win32
         %DEFINE ASM_CHIP_X86
      %ELIFIDN __OUTPUT_FORMAT__, win64
         %DEFINE ASM_CHIP_X64
      %ELSE
         %ERROR Neither ASM_CHIP_X86 nor ASM_CHIP_X64 are defined, and the target chip can't be inferred from the output format
      %ENDIF
   %ENDIF
%ENDIF

; -------------------------------------------------------------------------------

%IFDEF ASM_CHIP_X86
   %IFDEF ASM_CHIP_X64
      %ERROR Only one of ASM_CHIP_X86 and ASM_CHIP_X64 can be defined at the same time
   %ENDIF
%ENDIF

; -------------------------------------------------------------------------------

%IFDEF ASM_CHIP_X64
   %ASSIGN ASM_BITS 64
   BITS 64
   %DEFINE ASM_CHIP x64
%ELSE
   %ASSIGN ASM_BITS 32
   BITS 32
   %DEFINE ASM_CHIP x86
%ENDIF

; -------------------------------------------------------------------------------

%ASSIGN ASM_BYTES ASM_BITS / 8

; -------------------------------------------------------------------------------

%IFDEF ASM_CHIP_X64
   DEFAULT rel
%ENDIF

; -------------------------------------------------------------------------------
;  Name mangling :
;    - by default, on 32 bits archs, each function name will be prefixed by an
; underscore. On 64 bits archs, they won't.
;    - to prevent that behavior, either use -DASM_UNDERSCORE or -DASM_NO_UNDERSCORE
;    - ASM_MANGLING will be either empty, or an underscore
;    - ASM_GLOBAL and ASM_EXTERN are macros that wrap global & extern definition 
; and adds the underscore.
; -------------------------------------------------------------------------------

%IFNDEF ASM_UNDERSCORE
   %IFNDEF ASM_NO_UNDERSCORE
      %IFDEF ASM_CHIP_X64
         %DEFINE ASM_NO_UNDERSCORE
      %ELSE
         %DEFINE ASM_UNDERSCORE
      %ENDIF
   %ENDIF
%ENDIF

; -----------------------------------------------------------------------------

%IFDEF ASM_UNDERSCORE
   %IFDEF ASM_NO_UNDERSCORE
      %ERROR Only one of ASM_UNDERSCORE and ASM_NO_UNDERSCORE can be defined at the same time
   %ENDIF
%ENDIF

; -------------------------------------------------------------------------------

%IFDEF ASM_UNDERSCORE
   %DEFINE ASM_MANGLING(x) _ %+ x
%ELSE
   %DEFINE ASM_MANGLING(x) x
%ENDIF      

; -------------------------------------------------------------------------------

%MACRO ASM_GLOBAL 1
   GLOBAL ASM_MANGLING(%1)
   %DEFINE %1 ASM_MANGLING(%1)
%ENDMACRO

; -------------------------------------------------------------------------------

%MACRO ASM_EXTERN 1
   EXTERN ASM_MANGLING(%1)
   %DEFINE %1 ASM_MANGLING(%1)
%ENDMACRO

; -------------------------------------------------------------------------------
;  Read-only data section :
;    - use ASM_SECTION_RODATA to declare in the cleanest way a read only section
; -------------------------------------------------------------------------------

%MACRO ASM_SECTION_RODATA 0

   %IFDEF ASM_SECTION_RODATA_ALREADY_DEFINED
      %IFIDN __OUTPUT_FORMAT__, elf32
         SECTION .rodata
      %ELIFIDN __OUTPUT_FORMAT__, elf64
         SECTION .rdata
      %ELIFIDN __OUTPUT_FORMAT__, win32
         SECTION .rdata
      %ELIFIDN __OUTPUT_FORMAT__, win64
         SECTION .rdata
      %ELSE
         %ERROR Unsupported output format. Could not define ASM_SECTION_RODATA
      %ENDIF
   %ELSE      
      %IFIDN __OUTPUT_FORMAT__, elf32
         SECTION .rodata ALIGN=16
      %ELIFIDN __OUTPUT_FORMAT__, elf64
         SECTION .rdata ALIGN=16
      %ELIFIDN __OUTPUT_FORMAT__, win32
         SECTION .rdata ALIGN=16
      %ELIFIDN __OUTPUT_FORMAT__, win64
         SECTION .rdata ALIGN=16
      %ELSE
         %ERROR Unsupported output format. Could not define ASM_SECTION_RODATA
      %ENDIF
   %ENDIF
   
   %DEFINE ASM_SECTION_RODATA_ALREADY_DEFINED
   
%ENDMACRO

; -----------------------------------------------------------------------------
;  Argument position :
;    - This defines the address of the Xth argument
;    - ASM_STACK_OFFSET is defined later, and contain all the offsets that would
;  be a consequence of push/pop on the stack
;    - when pushing/popping data on the stack, or when handling the stack 
;  pointer, you must use the following macros : ASM_PUSH, ASM_POP, ASM_SUB_XSP, 
;  ASM_ADD_XSP, so that ASM_STACK_OFFSET is modified properly
; -----------------------------------------------------------------------------

%MACRO ASM_DEFINE_ARGUMENT 0
   %IF ASM_BITS=32
      ; win32/lin32 : just after call, [esp + 0] contains the return address, and [esp + 4] is the first argument
      %DEFINE ASM_ARGUMENT(x) esp + ((x) + 1) * ASM_BYTES + ASM_STACK_OFFSET
   %ELIFDEF ASM_OS_LIN
      ; lin64 : just after call, [esp + 0] contains the return address, and [esp + 8] is the seventh argument
      %DEFINE ASM_ARGUMENT(x) rsp + ((x) - 6 + 1) * ASM_BYTES + ASM_STACK_OFFSET
   %ELSE
      ; win64 : just after call, [esp + 32] contains the return address, and [esp + 40] is the fifth argument
      %DEFINE ASM_ARGUMENT(x) rsp + ((x) - 4 + 4 + 1) * ASM_BYTES + ASM_STACK_OFFSET
   %ENDIF   
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO ASM_DEFINE_VARIABLE 1
   %DEFINE ASM_VARIABLE(x) ybp + x + %1
%ENDMACRO   

; -----------------------------------------------------------------------------

%MACRO ASM_UNDEFINE_ARGUMENT 0
   %UNDEF ASM_ARGUMENT
%ENDMACRO   

; -----------------------------------------------------------------------------

%MACRO ASM_UNDEFINE_VARIABLE 0
   %UNDEF ASM_VARIABLE
%ENDMACRO   

; -----------------------------------------------------------------------------

%MACRO ASM_PUSH 1
   push %1
   %ASSIGN ASM_STACK_OFFSET ASM_STACK_OFFSET + ASM_BYTES
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO ASM_POP 1
   pop %1
   %ASSIGN ASM_STACK_OFFSET ASM_STACK_OFFSET - ASM_BYTES
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO ASM_ADD_XSP 1
   %IFNNUM %1
      %ERROR Stack pointer can only be offset by a numeric constant
   %ENDIF         
   %IF %1 / ASM_BYTES * ASM_BYTES != %1
      %ERROR Stack pointer can only be offset by a multiple of ASM_BYTES
   %ENDIF      
   add ASM_sp, %1
   %ASSIGN ASM_STACK_OFFSET ASM_STACK_OFFSET - %1
%ENDMACRO

; -----------------------------------------------------------------------------
   
%MACRO ASM_SUB_XSP 1
   %IFNNUM %1
      %ERROR Stack pointer can only be offset by a numeric constant
   %ENDIF
   %IF %1 / ASM_BYTES * ASM_BYTES != %1
      %ERROR Stack pointer can only be offset by a multiple of ASM_BYTES
   %ENDIF      
   sub ASM_sp, %1
   %ASSIGN ASM_STACK_OFFSET ASM_STACK_OFFSET + %1
%ENDMACRO

; -----------------------------------------------------------------------------
; ASM_ALLOCATE_VARIABLES "variables size", "mode" [, "base register"]
;  - defines ASM_VARIABLES_SIZE to the total size in bytes taken by the variables
; stored on the stack
;  - "mode" can be either one of "aligned" or "unaligned"
;  - if "mode" is "aligned", you can provide a "base register" for the variable
; area on the stack. Else, xbp will be used. Additionally, ASM_ALIGNED_VARIABLES
; will be defined.
;  - if "mode" is "unaligned", the "base register", if provided, will be ignored.
; xsp will be used.
; -----------------------------------------------------------------------------

%MACRO ASM_ALLOCATE_VARIABLES 2-3
   %UNDEF ASM_ALIGNED_VARIABLES
   %ASSIGN ASM_VARIABLES_SIZE %1
   %IF ASM_VARIABLES_SIZE > 0
      %IFIDNI %2, aligned
         %DEFINE ASM_ALIGNED_VARIABLES
         %IF %0 > 2
            %DEFINE ybp %3
         %ELSE
            %DEFINE ybp ASM_xbp
         %ENDIF
         ASM_SUB_XSP ASM_VARIABLES_SIZE + 16
         lea ybp, [ASM_sp + 16]
         and ybp, ~15
      %ELIFIDNI %2, unaligned
         %DEFINE ybp ASM_sp
         ASM_SUB_XSP ASM_VARIABLES_SIZE
      %ELSE
         %ERROR ASM_ALLOCATE_VARIABLES only support "aligned" and "unaligned" as second argument
      %ENDIF
      ASM_DEFINE_VARIABLE 0
   %ENDIF
%ENDMACRO   

; -----------------------------------------------------------------------------

%MACRO ASM_FREE_VARIABLES 0
   %IF ASM_VARIABLES_SIZE > 0
      ASM_UNDEFINE_VARIABLE
      %IFDEF ASM_ALIGNED_VARIABLES
         ASM_ADD_XSP ASM_VARIABLES_SIZE + 16
      %ELSE
         ASM_ADD_XSP ASM_VARIABLES_SIZE
      %ENDIF
   %ENDIF
%ENDMACRO
   
; -----------------------------------------------------------------------------
;  Register redefinition :
;    - registers are named rX (default size), and fixed size registers derive
;  from them : rXb, rXw, rXd, rXq
;    - only 7 register are guaranteed to be defined as such : rX, X=0..6.
;    - rX, X=7..12 will be defined only when ASM_CHIP_X64 is defined
;    - Depending on the architecture, rX won't always be aliased on the same 
;  register
;    - Register index isn't random. r0 is meant to be filled with the first
;  argument value. This holds true for r0..r5. 
;    - rXa for X=0..5 indicates where the Xth argument, if present, is (on the
;  stack, or in a register). rXa is a read-only register, because it actually
;  may not always be a register
; -----------------------------------------------------------------------------

; -----------------------------------------------------------------------------
; But first, we will create sized register aliases, to create a more consistent
; nomenclature than the official one. For example, rax, eax, ax and al are the
; 64bits, 32bits, 16bits and 8bits version of the same register. For consistency,
; we also create raxq, eaxq, raxd, eaxd, raxw, eaxw, raxb, eaxb. We do the same
; for rax/eax, rbx/ebx, rcx/ecx, rdx/edx, rsp/esp, rbp/ebp, rsi/esi and rdi/edi.
; -----------------------------------------------------------------------------

%MACRO ASM_DEFINE_REGISTER_SIZED 2
   %DEFINE r%1q r%1
   %DEFINE e%1q r%1
   %DEFINE r%1d e%1
   %DEFINE e%1d e%1
   %DEFINE r%1w %1
   %DEFINE e%1w %1
   %DEFINE r%1b %2
   %DEFINE e%1b %2
%ENDMACRO

; -----------------------------------------------------------------------------

ASM_DEFINE_REGISTER_SIZED ax, al
ASM_DEFINE_REGISTER_SIZED bx, bl
ASM_DEFINE_REGISTER_SIZED cx, cl
ASM_DEFINE_REGISTER_SIZED dx, dl
ASM_DEFINE_REGISTER_SIZED si, sil
ASM_DEFINE_REGISTER_SIZED di, sil
ASM_DEFINE_REGISTER_SIZED bp, dbp

; -----------------------------------------------------------------------------
; Then, we create a macro that will alias ASM_X(q|d|w|b) (with X a number) to the 
; corresponding sized versions, and ASM_X to the native sized version (depending
; on ASM_BITS
; -----------------------------------------------------------------------------

%MACRO ASM_DEFINE_REGISTER 5-6
   %DEFINE ASM_%1q %2
   %DEFINE ASM_%1d %3
   %DEFINE ASM_%1w %4
   %DEFINE ASM_%1b %5
   %IF ASM_BITS = 32
      %DEFINE ASM_%1 %3
   %ELSE
      %DEFINE ASM_%1 %2
   %ENDIF
%ENDMACRO

; -----------------------------------------------------------------------------
; There lies most of the portability behind this header. If you were to invent
; a new architecture, most of your modifications would be here.
; Basically, we define, according to ASM_BITS and ASM_CHIP, the alias between
; rX and real registers, how many registers are available, the preferred
; allocation index, which registers are used to pass arguments, their order,
; which registers are volatile (caller-saved)
; -----------------------------------------------------------------------------

%IF ASM_BITS=32
   ASM_DEFINE_REGISTER  0, UNDEF,  eax,    ax,    al
   ASM_DEFINE_REGISTER  1, UNDEF,  ecx,    cx,    cl
   ASM_DEFINE_REGISTER  2, UNDEF,  edx,    dx,    dl
   ASM_DEFINE_REGISTER  3, UNDEF,  ebx,    bx,    bl
   ASM_DEFINE_REGISTER  4, UNDEF,  esi,    si, UNDEF
   ASM_DEFINE_REGISTER  5, UNDEF,  edi,    di, UNDEF
   ASM_DEFINE_REGISTER  6, UNDEF,  ebp,    bp, UNDEF
   %ASSIGN ASM_VOLATILE_REGISTERS_MASK 0x0007
   %ASSIGN ASM_ARGUMENT_REGISTERS_MASK 0x0000
   %ASSIGN ASM_XAX_INDEX 0
   %ASSIGN ASM_XBX_INDEX 3
   %ASSIGN ASM_XCX_INDEX 1
   %ASSIGN ASM_XDX_INDEX 2
   %ASSIGN ASM_XSI_INDEX 4
   %ASSIGN ASM_XDI_INDEX 5
   %ASSIGN ASM_XBP_INDEX 6
   %ASSIGN ASM_MAX_REGISTERS 7
   ASM_DEFINE_REGISTER sp, UNDEF,  esp, UNDEF, UNDEF
%ELSE
   %IFDEF ASM_OS_LIN
      ASM_DEFINE_REGISTER  0,   rdi,  edi,    di,   dil
      ASM_DEFINE_REGISTER  1,   rsi,  esi,    si,   sil
      ASM_DEFINE_REGISTER  2,   rdx,  edx,    dx,    dl
      ASM_DEFINE_REGISTER  3,   rcx,  ecx,    cx,    cl
      ASM_DEFINE_REGISTER  4,    r8,  r8d,   r8w,   r8b
      ASM_DEFINE_REGISTER  5,    r9,  r9d,   r9w,   r9b
      ASM_DEFINE_REGISTER  6,   rax,  eax,    ax,    al
      %ASSIGN ASM_VOLATILE_REGISTERS_MASK 0x007F
      %ASSIGN ASM_ARGUMENT_REGISTERS_MASK 0x003F
      %ASSIGN ASM_XAX_INDEX 6
      %ASSIGN ASM_XBX_INDEX 7
      %ASSIGN ASM_XCX_INDEX 3
      %ASSIGN ASM_XDX_INDEX 2
      %ASSIGN ASM_XSI_INDEX 1
      %ASSIGN ASM_XDI_INDEX 0
      %ASSIGN ASM_XBP_INDEX 8
      %ASSIGN ASM_2_ALIAS_INDEX 11
      %ASSIGN ASM_3_ALIAS_INDEX 12
   %ELSE
      ASM_DEFINE_REGISTER  0,   rcx,  ecx,    cx,    cl
      ASM_DEFINE_REGISTER  1,   rdx,  edx,    dx,    dl
      ASM_DEFINE_REGISTER  2,    r8,  r8d,   r8w,   r8b
      ASM_DEFINE_REGISTER  3,    r9,  r9d,   r9w,   r9b
      ASM_DEFINE_REGISTER  4,   rax,  eax,    ax,    al
      ASM_DEFINE_REGISTER  5,   rdi,  edi,    di,   dil
      ASM_DEFINE_REGISTER  6,   rsi,  esi,    si,   sil
      %ASSIGN ASM_VOLATILE_REGISTERS_MASK 0x001F
      %ASSIGN ASM_ARGUMENT_REGISTERS_MASK 0x000F
      %ASSIGN ASM_XAX_INDEX 4
      %ASSIGN ASM_XBX_INDEX 7
      %ASSIGN ASM_XCX_INDEX 0
      %ASSIGN ASM_XDX_INDEX 1
      %ASSIGN ASM_XSI_INDEX 6
      %ASSIGN ASM_XDI_INDEX 5
      %ASSIGN ASM_XBP_INDEX 8
   %ENDIF   
   ASM_DEFINE_REGISTER  7,   rbx,  ebx,    bx,    bl
   ASM_DEFINE_REGISTER  8,   rbp,  ebp,    bp,   bpl
   ASM_DEFINE_REGISTER  9,   r10, r10d,  r10w,  r10b
   ASM_DEFINE_REGISTER 10,   r11, r11d,  r11w,  r11b
   ASM_DEFINE_REGISTER 11,   r12, r12d,  r12w,  r12b
   ASM_DEFINE_REGISTER 12,   r13, r13d,  r13w,  r13b
   %ASSIGN ASM_MAX_REGISTERS 13
   ASM_DEFINE_REGISTER sp,   rsp,  esp, UNDEF, UNDEF
%ENDIF

; -----------------------------------------------------------------------------
; Finally, here is a macro that aliases an argument/temporary name to a register
; -----------------------------------------------------------------------------

%MACRO ASM_DEFINE_TEMPORARY_REGISTER 2
   %IFNUM %1
      %DEFINE t%1q ASM_%2q
      %DEFINE t%1d ASM_%2d
      %DEFINE t%1w ASM_%2w
      %DEFINE t%1b ASM_%2b
      %DEFINE t%1  ASM_%2
   %ELSE
      %DEFINE %1q ASM_%2q
      %DEFINE %1d ASM_%2d
      %DEFINE %1w ASM_%2w
      %DEFINE %1b ASM_%2b
      %DEFINE %1  ASM_%2
   %ENDIF      
   %ASSIGN ASM_TEMPORARY_REGISTERS_MASK ASM_TEMPORARY_REGISTERS_MASK | (1 << (%2))
   %ASSIGN ASM_ASSIGNED_REGISTERS_MASK ASM_ASSIGNED_REGISTERS_MASK | (1 << (%2))
%ENDMACRO   

; -----------------------------------------------------------------------------
; Stack handling : 
;  - set of function to reserve space on the stack, to align stack, and so on
;  - real stack pointer depends on architecture. you should never have to use it 
; explicitely
;  - there are two ways for aligning the stack :
;    - if you don't need to access the function args after the function protocol,
; and if you have one free additional register that can be trashed, then alignment
; don't use any register (but you lose the ability to get function arguments)
;    - if you do, then ebp is reserved (in addition to esp). ebp will be aligned,
; while esp won't
;  - whatever the method : ASM_ARGUMENT(x) gives the address of the xth argument 
; (starting from 0) and ASM_VARIABLE(x) gives the address of the xth aligned byte. 
; 
; -----------------------------------------------------------------------------

; -----------------------------------------------------------------------------
;  Argument functions :
;    - Indicates how many registers we plan to use, and consequently how many
;  non volatile registers needs to be saved, and which ones. Those registers
;  will later be automatically restored by a call to RETURN
;    - We can also indicates how much place we want to reserve on the stack (in
;  addition to the place taken by saved registers)
;    - Finally, when some arguments are used in read-only and we are a bit 
;  short on registers, allow to indicate that, and thus to free the related 
;  register for common purpose use. If the arguments was stored in a register 
;  (ie if we're 64 bits), we alias its name on one of the unused registers (r10
;  - r15). 
; -----------------------------------------------------------------------------

%MACRO ASM_DECLARE_ANY_TEMPORARY 1-2
   %ASSIGN %%I 0
   %REP 32
      %IF ((ASM_ASSIGNED_REGISTERS_MASK >> %%I) & 1 == 0) && ((ASM_ARGUMENT_REGISTERS_MASK & ((1 << %1) - 1)) >> %%I & 1 == 0)
         %IF %0 > 1
            ASM_DEFINE_TEMPORARY_REGISTER %2, %%I
         %ENDIF
         ASM_DEFINE_TEMPORARY_REGISTER ASM_NUM_TEMPORARY_REGISTERS, %%I
         %ASSIGN ASM_NUM_TEMPORARY_REGISTERS ASM_NUM_TEMPORARY_REGISTERS + 1
         %EXITREP
      %ENDIF
      %ASSIGN %%I %%I + 1
   %ENDREP
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO ASM_DECLARE_SPECIFIC_TEMPORARY 2
   %ASSIGN %%INDEX ASM_%1_INDEX
   ASM_DEFINE_TEMPORARY_REGISTER %2, %%INDEX
   %ASSIGN ASM_NUM_TEMPORARY_REGISTERS ASM_NUM_TEMPORARY_REGISTERS + 1
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO ASM_DECLARE_TEMPORARY 1-2

   %IF %0 = 1
      ASM_DECLARE_ANY_TEMPORARY %1
   %ELIFIDNI %2, xax
      ASM_DECLARE_SPECIFIC_TEMPORARY XAX, xax
   %ELIFIDNI %2, xbx
      ASM_DECLARE_SPECIFIC_TEMPORARY XBX, xbx
   %ELIFIDNI %2, xcx
      ASM_DECLARE_SPECIFIC_TEMPORARY XCX, xcx
   %ELIFIDNI %2, xdx
      ASM_DECLARE_SPECIFIC_TEMPORARY XDX, xdx
   %ELIFIDNI %2, xdi
      ASM_DECLARE_SPECIFIC_TEMPORARY XDI, xdi
   %ELIFIDNI %2, xsi
      ASM_DECLARE_SPECIFIC_TEMPORARY XSI, xsi
   %ELIFIDNI %2, xbp
      ASM_DECLARE_SPECIFIC_TEMPORARY XBP, xbp
   %ELSE
      ASM_DECLARE_ANY_TEMPORARY %1, %2
   %ENDIF

%ENDMACRO  

; -----------------------------------------------------------------------------

%MACRO ASM_ASSIGN_ARGUMENT_REGISTER 2-3

   %ASSIGN %%INDEX 0
   
   ; If the argument is passed by register, and if that register isn't assigned, 
   ; set %%INDEX to that register.
   ; Else, find a proper register and store its index into %%INDEX, and if the 
   ; argument was passed as a register, flag it for further copy
   
   %IF ((ASM_ARGUMENT_REGISTERS_MASK >> %1) & 1 == 1) && ((ASM_ASSIGNED_REGISTERS_MASK >> %1) & 1 == 0)
      %ASSIGN %%INDEX %1
   %ELSE      
      %REP ASM_MAX_REGISTERS
         %IF ((ASM_ASSIGNED_REGISTERS_MASK >> %%INDEX) & 1 == 0) && ((ASM_ARGUMENT_REGISTERS_MASK >> %%INDEX) & 1 == 0)
            %EXITREP
         %ENDIF
         %ASSIGN %%INDEX %%INDEX + 1
      %ENDREP
      
      %IF (ASM_ARGUMENT_REGISTERS_MASK >> %1) & 1 == 1
         %ASSIGN ASM_COPIED_REGISTER_SOURCE_MASK ASM_COPIED_REGISTER_SOURCE_MASK | (%1 << (4 * ASM_NUM_COPIED_REGISTERS))
         %ASSIGN ASM_COPIED_REGISTER_DESTINATION_MASK ASM_COPIED_REGISTER_DESTINATION_MASK | (%%INDEX << (4 * ASM_NUM_COPIED_REGISTERS))
         %ASSIGN ASM_NUM_COPIED_REGISTERS ASM_NUM_COPIED_REGISTERS + 1
      %ENDIF
      
   %ENDIF
   
   ; Alias the name of the argument to the free register
   
   ASM_XDEFINE_CONCATENATION %2, ASM_, %%INDEX
   ASM_XDEFINE_CONCATENATION %2b, ASM_, %%INDEX, b
   ASM_XDEFINE_CONCATENATION %2w, ASM_, %%INDEX, w
   ASM_XDEFINE_CONCATENATION %2d, ASM_, %%INDEX, d
   ASM_XDEFINE_CONCATENATION %2q, ASM_, %%INDEX, q

   ; Mark the register as used

   %ASSIGN ASM_ASSIGNED_REGISTERS_MASK ASM_ASSIGNED_REGISTERS_MASK | (1 << %%INDEX)

   ; Mark the register as loaded, if the argument was asked to be loaded
   ; and if the argument wasn't already in a register

   %IFIDNI %3, load
      %IF ((ASM_ARGUMENT_REGISTERS_MASK >> %1) & 1 == 0) || ((ASM_ASSIGNED_REGISTERS_MASK >> %1) & 1 == 0)
         %ASSIGN ASM_ARGUMENTS_TO_BE_LOADED_REGISTER_MASK ASM_ARGUMENTS_TO_BE_LOADED_REGISTER_MASK | (%%INDEX << (4 * ASM_NUM_ARGUMENTS_TO_BE_LOADED))
         %ASSIGN ASM_ARGUMENTS_TO_BE_LOADED_POSITION_MASK ASM_ARGUMENTS_TO_BE_LOADED_POSITION_MASK | (%1 << (4 * ASM_NUM_ARGUMENTS_TO_BE_LOADED))
         %ASSIGN ASM_NUM_ARGUMENTS_TO_BE_LOADED ASM_NUM_ARGUMENTS_TO_BE_LOADED + 1
      %ENDIF
   %ENDIF

%ENDMACRO   

; -----------------------------------------------------------------------------

%MACRO ASM_DECLARE_ARGUMENT 1-*

   ; if we don't need the argument to be loaded into a register, and
   ; if the argument isn't already in a register, alias the argument name
   ; to its stack position (do note the subtle use of %XDEFINE to force 
   ; the dereferencing of ASM_NUM_ARGUMENTS now instead of later)
   ; additionally : don't try to make a macro and pass [ASM_ARGUMENT(%%K)] to it.
   ; as soon as it becomes a macro parameter, it gets entirely expanded,
   ; and so does ASM_STACK_OFFSET, which breaks the whole purpose of ASM_ARGUMENT
   ; if ASM_STACK_OFFSET is incremented afterwards
   
   %IFNIDNI %2, load
      %IF (ASM_ARGUMENT_REGISTERS_MASK >> ASM_NUM_ARGUMENTS) & 1 == 0
         %XDEFINE %%K ASM_NUM_ARGUMENTS
         %DEFINE %1q qword [ASM_ARGUMENT(%%K)]
         %DEFINE %1d dword [ASM_ARGUMENT(%%K)]
         %DEFINE %1w word [ASM_ARGUMENT(%%K)]
         %DEFINE %1b byte [ASM_ARGUMENT(%%K)]
         %DEFINE %1 [ASM_ARGUMENT(%%K)]
      %ENDIF
   %ENDIF

   ; if the argument is passed by register, or if a load is requested
   ; we must assign the argument to that register. 
   ; If the register was reserved as a temporary one,
   ; we must additionnaly allocate another register, and flag it
   ; so that its value will be properly set once registers are pushed
   ; on the stack
   
   %IFIDNI %2, load
      ASM_ASSIGN_ARGUMENT_REGISTER ASM_NUM_ARGUMENTS, %1, load
   %ELIF (ASM_ARGUMENT_REGISTERS_MASK >> ASM_NUM_ARGUMENTS) & 1 == 1
      ASM_ASSIGN_ARGUMENT_REGISTER ASM_NUM_ARGUMENTS, %1
   %ENDIF
   
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO ASM_SAVE_REGISTERS 0
   %ASSIGN %%I 0
   %REP ASM_MAX_REGISTERS
      %IF (ASM_VOLATILE_REGISTERS_MASK >> %%I) & 1 == 0
         %IF (ASM_ASSIGNED_REGISTERS_MASK >> %%I) & 1 == 1
            ASM_PUSH ASM_ %+ %%I
         %ENDIF
      %ENDIF
      %ASSIGN %%I %%I + 1
   %ENDREP
%ENDMACRO  

; -----------------------------------------------------------------------------

%MACRO ASM_LOAD_REGISTERS 0
   %ASSIGN %%I ASM_MAX_REGISTERS
   %REP ASM_MAX_REGISTERS
      %ASSIGN %%I %%I - 1
      %IF (ASM_VOLATILE_REGISTERS_MASK >> %%I) & 1 == 0
         %IF (ASM_ASSIGNED_REGISTERS_MASK >> %%I) & 1 == 1
            ASM_POP ASM_ %+ %%I
         %ENDIF
      %ENDIF
   %ENDREP
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO STACK 0-*

   ; Push a defines context on the defines stack. All further defines will be destroyed
   ; once %endscope is called
   
   %SCOPE

   ; That macro defines ASM_ARGUMENT. It is defined by default, but can be undefined if
   ; the user want to align the stack without losing a register. 

   ASM_DEFINE_ARGUMENT

   ; Initialize some variables
   
   ; ASM_STACK_OFFSET indicates by how much the stack pointer was moved from its original position
   ; It allows access to function arguments that are on the stack
      
   %ASSIGN ASM_STACK_OFFSET 0
   
   ; ASM_NUM_ARGUMENTS contains the number of arguments (registers or on the stack) for the function
   
   %ASSIGN ASM_NUM_ARGUMENTS 0
   
   ; ASM_NUM_TEMPORARY_REGISTERS constains the number of temporary registers requested for the function
   ; ASM_TEMPORARY_REGISTERS_MASK indicates which registers are used as temporaries
   ; ASM_ASSIGNED_REGISTERS_MASK indicates which registers are used
   
   %ASSIGN ASM_NUM_TEMPORARY_REGISTERS 0
   %ASSIGN ASM_TEMPORARY_REGISTERS_MASK 0
   %ASSIGN ASM_ASSIGNED_REGISTERS_MASK 0
   
   ; ASM_NUM_ARGUMENTS_TO_BE_LOADED contains the number of arguments that must be loaded from the stack into a register
   ; ASM_ARGUMENTS_TO_BE_LOADED_POSITION_MASK indicates which argument is the xth argument to be loaded
   ; ASM_ARGUMENTS_TO_BE_LOADED_REGISTER_MASK indicates into which register the xth argument to be loaded will go

   %ASSIGN ASM_NUM_ARGUMENTS_TO_BE_LOADED 0
   %ASSIGN ASM_ARGUMENTS_TO_BE_LOADED_REGISTER_MASK 0
   %ASSIGN ASM_ARGUMENTS_TO_BE_LOADED_POSITION_MASK 0

   ; ASM_NUM_COPIED_REGISTERS contains the number of explicitely named temporary registers that are used to stored function arguments
   ; When that happens, the argument must be put in another register (note : we could put it on the stack, but we always have
   ; enough registers to store it in a register, because - at the moment - argument in register equals 64bits equals plenty of regs
   ; ASM_COPIED_REGISTER_SOURCE_MASK indicates into which register the xth forced argument register was originally stored
   ; ASM_COPIED_REGISTER_DESTINATION_MASK indicates into which register the xth forced argument will be stored
      
   %ASSIGN ASM_NUM_COPIED_REGISTERS 0
   %ASSIGN ASM_COPIED_REGISTER_SOURCE_MASK 0
   %ASSIGN ASM_COPIED_REGISTER_DESTINATION_MASK 0
   
   ; Parse the arguments. We expect something like : [[[num[[,aligned[,noreg|regname]],var1[, size1], var2...]]],,][tmp1, tmp2...,,]]arg1[,*],arg2[,*]...]
   ; First, we count the number of double commas, and their position.

   %ASSIGN %%FIRST_DOUBLE_COMMA -1
   %ASSIGN %%SECOND_DOUBLE_COMMA -1
   %ASSIGN %%POSITION 0
   %REP %0
      %IFNUM %1
      %ELIFID %1
      %ELIFIDNI %1, *
      %ELSE
         %IF %%FIRST_DOUBLE_COMMA < 0
            %ASSIGN %%FIRST_DOUBLE_COMMA %%POSITION + 1
         %ELIF %%SECOND_DOUBLE_COMMA < 0
            %ASSIGN %%SECOND_DOUBLE_COMMA %%POSITION + 1
         %ELSE
            %ERROR Syntax error : Only two double commas are allowed
         %ENDIF
      %ENDIF
      %ASSIGN %%POSITION %%POSITION + 1
      %ROTATE 1
   %ENDREP
   
   ; Find the start/stop of each groups (var, tmp, arg)
   
   %ASSIGN %%START_VARIABLES -1
   %ASSIGN %%START_TEMPORARIES -1
   %ASSIGN %%START_ARGUMENTS -1
   %ASSIGN %%STOP_VARIABLES -1
   %ASSIGN %%STOP_TEMPORARIES -1
   %ASSIGN %%STOP_ARGUMENTS -1
   
   %IFIDNI %1, stack
      %ASSIGN %%START_VARIABLES 1
      %IF %%SECOND_DOUBLE_COMMA >= 0
         %ASSIGN %%START_ARGUMENTS %%SECOND_DOUBLE_COMMA
         %ASSIGN %%STOP_ARGUMENTS %0
         %ASSIGN %%START_TEMPORARIES %%FIRST_DOUBLE_COMMA
         %ASSIGN %%STOP_TEMPORARIES %%START_ARGUMENTS
         %ASSIGN %%STOP_VARIABLES %%START_TEMPORARIES
      %ELIF %%FIRST_DOUBLE_COMMA >= 0
         %ASSIGN %%START_ARGUMENTS %%FIRST_DOUBLE_COMMA
         %ASSIGN %%STOP_ARGUMENTS %0
         %ASSIGN %%STOP_VARIABLES %%START_ARGUMENTS
      %ELSE
         %ASSIGN %%STOP_VARIABLES %0
      %ENDIF
   %ELSE
      %IF %%FIRST_DOUBLE_COMMA >= 0
         %ASSIGN %%START_ARGUMENTS %%FIRST_DOUBLE_COMMA
         %ASSIGN %%STOP_ARGUMENTS %0
         %ASSIGN %%START_TEMPORARIES 0
         %ASSIGN %%STOP_TEMPORARIES %%START_ARGUMENTS
      %ELSE
         %ASSIGN %%START_ARGUMENTS 0
         %ASSIGN %%STOP_ARGUMENTS %0
      %ENDIF
   %ENDIF
   
   ; Check whether the stack is aligned. If it is, and if noreg wasn't specified, reserve a stack register (xbp per default, user specified otherwise)

   %ASSIGN %%VARIABLES_PROLOGUE 0
   %IFIDNI %1, stack
      %IFIDNI %2, aligned
         %IFNIDNI %3, noreg
            %ASSIGN %%VARIABLES_PROLOGUE 2
            %IFIDNI %3, xax
               ASM_DECLARE_TEMPORARY unused, xax
            %ELIFIDNI %3, xbx
               ASM_DECLARE_TEMPORARY unused, xbx
            %ELIFIDNI %3, xcx
               ASM_DECLARE_TEMPORARY unused, xcx
            %ELIFIDNI %3, xdx
               ASM_DECLARE_TEMPORARY unused, xdx
            %ELIFIDNI %3, xsi
               ASM_DECLARE_TEMPORARY unused, xsi
            %ELIFIDNI %3, xdi
               ASM_DECLARE_TEMPORARY unused, xdi
            %ELSE
               ASM_DECLARE_TEMPORARY unused, xbp
               %ASSIGN %%VARIABLES_PROLOGUE 1
            %ENDIF
         %ELSE
            %ERROR noreg isnt supported at the moment : aligned stack requires a register
         %ENDIF
      %ENDIF
   %ENDIF
   
   ; Parse the number of arguments (it is needed to allocated unforced temporary registers)
   
   %ASSIGN %%POSITION 0
   %ASSIGN %%NUM_ARGUMENTS 0
   %REP %0
      %IF %%POSITION >= %%START_ARGUMENTS && %%POSITION < %%STOP_ARGUMENTS
         %IFNIDNI %1, *
            %IFID %1
               %ASSIGN %%NUM_ARGUMENTS %%NUM_ARGUMENTS + 1
            %ENDIF
         %ENDIF
      %ENDIF
      %ASSIGN %%POSITION %%POSITION + 1
      %ROTATE 1
   %ENDREP
   
   ; Allocate temporary registers
   
   %ASSIGN %%POSITION 0
   %REP %0
      %IF %%POSITION >= %%START_TEMPORARIES && %%POSITION < %%STOP_TEMPORARIES
         %IFID %1
            ASM_DECLARE_TEMPORARY %%NUM_ARGUMENTS, %1
         %ENDIF
      %ENDIF
      %ASSIGN %%POSITION %%POSITION + 1
      %ROTATE 1
   %ENDREP
   
   ; Allocate arguments registers
   
   %ASSIGN %%POSITION 0
   %REP %0
      %IF %%POSITION >= %%START_ARGUMENTS && %%POSITION < %%STOP_ARGUMENTS
         %IFID %1
            %IFIDNI %2, *
               ASM_DECLARE_ARGUMENT %1, load
            %ELSE
               ASM_DECLARE_ARGUMENT %1
            %ENDIF
            %ASSIGN ASM_NUM_ARGUMENTS ASM_NUM_ARGUMENTS + 1
         %ENDIF
      %ENDIF
      %ASSIGN %%POSITION %%POSITION + 1
      %ROTATE 1
   %ENDREP
   
   ; Now that we know exactly which registers will be used, save callee-saved registers on the stack

   ASM_SAVE_REGISTERS
   
   ; Compute the size to allocate on the stack for variables
   
   %ASSIGN %%TOTAL_VARIABLES_SIZE 0
   %ASSIGN %%POSITION 0
   %REP %0
      %IF %%POSITION >= %%START_VARIABLES + %%VARIABLES_PROLOGUE && %%POSITION < %%STOP_VARIABLES
         %IFID %1
            %IFNUM %2
               %ASSIGN %%TOTAL_VARIABLES_SIZE %%TOTAL_VARIABLES_SIZE + %2
            %ELSE
               %ASSIGN %%TOTAL_VARIABLES_SIZE %%TOTAL_VARIABLES_SIZE + ASM_BYTES
            %ENDIF
         %ENDIF
      %ENDIF
      %ASSIGN %%POSITION %%POSITION + 1
      %ROTATE 1
   %ENDREP
   
   %IF %%TOTAL_VARIABLES_SIZE < 0
      %ERROR Something has gone wrong with the total size variables. My bet is that you gave a negative size
   %ENDIF 
   
   ; Now that we know how much stack we need, allocate it
   
   %IF %%START_VARIABLES >= 0
      %IFIDNI %2, aligned
         %IF %%VARIABLES_PROLOGUE = 2
            ASM_ALLOCATE_VARIABLES %%TOTAL_VARIABLES_SIZE, aligned, %3
         %ELSE
            ASM_ALLOCATE_VARIABLES %%TOTAL_VARIABLES_SIZE, aligned, xbp
         %ENDIF
      %ELSE
         ASM_ALLOCATE_VARIABLES %%TOTAL_VARIABLES_SIZE, unaligned
      %ENDIF
   %ELSE
      ASM_ALLOCATE_VARIABLES %%TOTAL_VARIABLES_SIZE, unaligned
   %ENDIF
   
   ; Now, create the aliases between variable addresses and variable names.
   ; Note, once again, the use of xdefine
   
   %ASSIGN %%POSITION 0
   %ASSIGN %%CURRENT_VARIABLES_SIZE 0
   %REP %0
      %IF %%POSITION >= %%START_VARIABLES + %%VARIABLES_PROLOGUE && %%POSITION < %%STOP_VARIABLES
         %IFID %1
            %XDEFINE %%CURRENT_VARIABLES_SIZE_FREEZED %%CURRENT_VARIABLES_SIZE
            %XDEFINE p%1(x) [ASM_VARIABLE(%%CURRENT_VARIABLES_SIZE_FREEZED) + x]
            %XDEFINE %1q qword [ASM_VARIABLE(%%CURRENT_VARIABLES_SIZE_FREEZED)]
            %XDEFINE %1d dword [ASM_VARIABLE(%%CURRENT_VARIABLES_SIZE_FREEZED)]
            %XDEFINE %1w word [ASM_VARIABLE(%%CURRENT_VARIABLES_SIZE_FREEZED)]
            %XDEFINE %1b byte [ASM_VARIABLE(%%CURRENT_VARIABLES_SIZE_FREEZED)]
            %XDEFINE %1 [ASM_VARIABLE(%%CURRENT_VARIABLES_SIZE_FREEZED)]
            
            %IFNUM %2
               %ASSIGN %%CURRENT_VARIABLES_SIZE %%CURRENT_VARIABLES_SIZE + %2
            %ELSE
               %ASSIGN %%CURRENT_VARIABLES_SIZE %%CURRENT_VARIABLES_SIZE + ASM_BYTES
            %ENDIF
         %ENDIF
      %ENDIF
      %ASSIGN %%POSITION %%POSITION + 1
      %ROTATE 1
   %ENDREP
   
   ; We're almost done : load arguments that needs to be loaded
      
   %ASSIGN %%I 0
   %REP ASM_NUM_ARGUMENTS_TO_BE_LOADED
      %ASSIGN %%REGISTER (ASM_ARGUMENTS_TO_BE_LOADED_REGISTER_MASK >> (4 * %%I)) & 0xF
      %ASSIGN %%POSITION (ASM_ARGUMENTS_TO_BE_LOADED_POSITION_MASK >> (4 * %%I)) & 0xF
      mov ASM_ %+ %%REGISTER, [ASM_ARGUMENT(%%POSITION)]
      %ASSIGN %%I %%I + 1
   %ENDREP

   ; We're really close : move arguments whose register was reserved into another register
      
   %ASSIGN %%I 0
   %REP ASM_NUM_COPIED_REGISTERS
      %ASSIGN %%SOURCE (ASM_COPIED_REGISTER_SOURCE_MASK >> (4 * %%I)) & 0xF
      %ASSIGN %%DESTINATION (ASM_COPIED_REGISTER_DESTINATION_MASK >> (4 * %%I)) & 0xF
      mov ASM_ %+ %%DESTINATION, ASM_ %+ %%SOURCE
      %ASSIGN %%I %%I + 1
   %ENDREP      
   
   ; Yeah ! Finished
   
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO UNSTACK 0
   ASM_FREE_VARIABLES
   ASM_LOAD_REGISTERS
   
   %ENDSCOPE
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO RETURN 0
   UNSTACK
   ret
%ENDMACRO

