; -----------------------------------------------------------------------------
;  Architecture definition :
;    - only one of _CHIP_X86 and _CHIP_X64 can be defined
;    - only one of _OS_LIN and _OS_WIN can be defined
;    - _CHIP will expand either to x86 or to x64.
;    - _OS will expand to either win or lin
;    - _BITS will expand to either 32 or 64
;    - architecture is inferred from output format, but can be forced through
;  external defines : either use -D_CHIP_X86 or -D_CHIP_X64, and -D_OS_WIN or
;  -D_OS_LIN
; -----------------------------------------------------------------------------

%IFIDN __OUTPUT_FORMAT__, elf64
   %DEFINE _OS_LIN
%ELIFIDN __OUTPUT_FORMAT__, elf32
   %DEFINE _OS_LIN
%ELIDIDN __OUTPUT_FORMAT__, elf
   %DEFINE _OS_LIN
%ENDIF      

; -----------------------------------------------------------------------------

%IFDEF _OS_WIN
   %UNDEF _OS_LIN
%ENDIF

%IFDEF _OS_LIN
   %UNDEF _OS_WIN
%ELSE
   %DEFINE _OS_WIN
%ENDIF

; -----------------------------------------------------------------------------

%IFDEF _OS_WIN
   %DEFINE _OS win
%ELSE
   %DEFINE _OS lin
%ENDIF      

; -----------------------------------------------------------------------------

%IFIDN __OUTPUT_FORMAT__, elf64
   %DEFINE _CHIP_X64
%ELIFIDN __OUTPUT_FORMAT__, win64
   %DEFINE _CHIP_X64
%ELIFIDN __OUTPUT_FORMAT__, x64
   %DEFINE _CHIP_X64
%ENDIF

; -----------------------------------------------------------------------------

%IFDEF _CHIP_X86
   %UNDEF _CHIP_X64
%ENDIF

%IFDEF _CHIP_X64
   %UNDEF _CHIP_X86
%ELSE
   %DEFINE _CHIP_X86
%ENDIF

; -----------------------------------------------------------------------------

%IFDEF _CHIP_X64
   %ASSIGN _BITS 64
   BITS 64
   %DEFINE _CHIP x64
%ELSE
   %ASSIGN _BITS 32
   BITS 32
   %DEFINE _CHIP x86
%ENDIF

%ASSIGN _BYTES _BITS / 8

; -----------------------------------------------------------------------------
;  Name mangling :
;    - by default, on 32 bits archs, each function name will be prefixed by a _
; and on 64 bits archs, they won't.
;    - to prevent that behavior, use either -D_UNDERSCORE or -D_NO_UNDERSCORE
;    - _GLOBAL and _EXTERN wraps global & extern definition and adds the _
; -----------------------------------------------------------------------------

%IFDEF _CHIP_X64
   %IFNDEF _UNDERSCORE
      %DEFINE _NO_UNDERSCORE
   %ELSE
      %UNDEF _NO_UNDERSCORE
   %ENDIF
%ELSE
   %IFNDEF _NO_UNDERSCORE
      %DEFINE _UNDERSCORE
   %ELSE
      %UNDEF _UNDERSCORE
   %ENDIF
%ENDIF

; -----------------------------------------------------------------------------

%IFDEF _UNDERSCORE
   %UNDEF _NO_UNDERSCORE
%ENDIF

; -----------------------------------------------------------------------------

%MACRO __GLOBAL 1
   %IFDEF _UNDERSCORE
      GLOBAL _%1
      %DEFINE %1 _%1
   %ELSE
      GLOBAL %1
   %ENDIF
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO _GLOBAL 1-*
   %IF %0 = 1
      __GLOBAL %1
   %ELSE
      %DEFINE __BASENAME__ %1
      %ROTATE 1
      %REP %0 - 1
         __GLOBAL __BASENAME__ %+ _ %+ %1
         %ROTATE 1
      %ENDREP
      %UNDEF __BASENAME__
   %ENDIF
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO _EXTERN 1
   %IFDEF _UNDERSCORE
      EXTERN _%1
      %DEFINE %1 _%1
   %ELSE
      EXTERN %1
   %ENDIF
%ENDMACRO

; -----------------------------------------------------------------------------
;  Read-only data section :
;    - use SECTION_RODATA to declare in the cleanest way a read only section
; -----------------------------------------------------------------------------

%MACRO SECTION_RODATA 0
   %IFIDN __OUTPUT_FORMAT__,elf
      SECTION .rodata ALIGN=16
   %ELIFIDN __OUTPUT_FORMAT__,elf32
      SECTION .rodata ALIGN=16
   %ELIFIDN __OUTPUT_FORMAT__,elf64
      SECTION .rdata ALIGN=16
   %ELIFIDN __OUTPUT_FORMAT__,win32
      SECTION .rdata ALIGN=16
   %ELIFIDN __OUTPUT_FORMAT__,win64
      SECTION .rdata ALIGN=16
   %ELSE
      SECTION .text ALIGN=16
   %ENDIF
%ENDMACRO

; -----------------------------------------------------------------------------
;  Argument position :
;    - This defines the address of the Xth argument
;    - _STACK_OFFSET is defined later, and contain all the offsets that would
;  be consequence of push/pop on the stack
;    - when pushing/popping data on the stack, or when handling the stack 
;  pointer, you must use the following macros : PUSH, POP, SUB, ADD, so that
;  _STACK_OFFSET is modified accordingly
; -----------------------------------------------------------------------------

%MACRO _DEFINE_ARGUMENT 0
   %IF _BITS=32
      %DEFINE _ARGUMENT(x) esp + ((x) + 1) * 4 + _STACK_OFFSET
   %ELIFDEF _OS_LIN
      %DEFINE _ARGUMENT(x) rsp + ((x) - 5) * 8 + _STACK_OFFSET
   %ELSE
      %DEFINE _ARGUMENT(x) rsp + ((x) + 5) * 8 + _STACK_OFFSET
   %ENDIF   
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO _DEFINE_VARIABLE 1
   %DEFINE _VARIABLE(x) ybp + x + %1
%ENDMACRO   

; -----------------------------------------------------------------------------

%MACRO _UNDEFINE_ARGUMENT 0
   %UNDEF _ARGUMENT
%ENDMACRO   

; -----------------------------------------------------------------------------

%MACRO _UNDEFINE_VARIABLE 0
   %UNDEF _VARIABLE
%ENDMACRO   

; -----------------------------------------------------------------------------

%MACRO _PUSH 1
   push %1
   %ASSIGN _STACK_OFFSET _STACK_OFFSET + _BYTES
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO _POP 1
   pop %1
   %ASSIGN _STACK_OFFSET _STACK_OFFSET - _BYTES
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO _ADDSP 1
   add x___sp, %1
   %IFNNUM %1
      %ERROR Stack pointer can only be offset by a numeric constant
   %ENDIF         
   %IF %1 / _BYTES * _BYTES != %1
      %ERROR Stack pointer can only be offset by a multiple of _BYTES         
   %ENDIF      
   %ASSIGN _STACK_OFFSET _STACK_OFFSET - %1
%ENDMACRO

; -----------------------------------------------------------------------------
   
%MACRO _SUBSP 1
   sub x___sp, %1
   %IFNNUM %1
      %ERROR Stack pointer can only be offset by a numeric constant
   %ENDIF
   %IF %1 / _BYTES * _BYTES != %1
      %ERROR Stack pointer can only be offset by a multiple of _BYTES         
   %ENDIF      
   %ASSIGN _STACK_OFFSET _STACK_OFFSET + %1
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO _ALLOCATE_VARIABLES 1-4
   %UNDEF _ALIGNED_VARIABLES
   %ASSIGN _VARIABLES_SIZE %1
   %IF _VARIABLES_SIZE > 0
      %IFIDNI %4, aligned
         %DEFINE _ALIGNED_VARIABLES
         %IFIDNI %3, noargs
            _UNDEFINE_ARGUMENT 
            %DEFINE ybp x___sp
            mov %2, ybp
            sub ybp, _VARIABLES_SIZE + 16
            and ybp, ~15
            mov [ybp], %2
            _DEFINE_VARIABLE 16
         %ELSE
            %DEFINE ybp %2
            _SUBSP _VARIABLES_SIZE + 16
            lea ybp, [x___sp + 16]
            and ybp, ~15
            _DEFINE_VARIABLE 0
         %ENDIF
      %ELSE
         %DEFINE ybp x___sp
         _SUBSP _VARIABLES_SIZE
         _DEFINE_VARIABLE 0
      %ENDIF
   %ENDIF
%ENDMACRO   

; -----------------------------------------------------------------------------

%MACRO _FREE_VARIABLES 0
   %IF _VARIABLES_SIZE > 0
      _UNDEFINE_VARIABLE
      %IFDEF _ALIGNED_VARIABLES
         %IFIDNI ybp, x___sp
            mov ybp, [ybp]
         %ELSE
            _ADDSP _VARIABLES_SIZE + 16
         %ENDIF
      %ELSE
         _ADDSP _VARIABLES_SIZE
      %ENDIF
   %ENDIF
%ENDMACRO
   
; -----------------------------------------------------------------------------
;  Register redefinition :
;    - registers are named rX (default size), and fixed size registers derive
;  from them : rXb, rXw, rXd, rXq
;    - only 7 register are defined as such : rX, X=0..6.
;    - Depending on the architecture, rX won't always be aliased on the same 
;  register
;    - Register index isn't random. r0 is meant to be filled with the first
;  argument value. This holds true for r0..r5. 
;    - rXa for X=0..5 indicates where the Xth argument, if present, is (on the
;  stack, or in a register). rXa is a read-only register, because it actually
;  may not always be a register
; -----------------------------------------------------------------------------

%MACRO _DEFINE_REGISTER 5-6
   %DEFINE x___%1q %2
   %DEFINE x___%1d %3
   %DEFINE x___%1w %4
   %DEFINE x___%1b %5
   %IF _BITS = 32
      %DEFINE x___%1 %3
   %ELSE
      %DEFINE x___%1 %2
   %ENDIF
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO _DEFINE_REGISTER_SIZED 2
   %DEFINE r%1q r%1
   %DEFINE e%1q r%1
   %DEFINE r%1d e%1
   %DEFINE e%1d e%1
   %DEFINE r%1w %1
   %DEFINE e%1w %1
   %DEFINE r%1b %2
   %DEFINE e%1b %2
%ENDMACRO

_DEFINE_REGISTER_SIZED ax, al
_DEFINE_REGISTER_SIZED bx, bl
_DEFINE_REGISTER_SIZED cx, cl
_DEFINE_REGISTER_SIZED dx, dl
_DEFINE_REGISTER_SIZED si, sil
_DEFINE_REGISTER_SIZED di, sil
_DEFINE_REGISTER_SIZED bp, dbp

; -----------------------------------------------------------------------------

%MACRO _XDEFINE_CATCON 3
   %XDEFINE %1%2 %3
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO _DEFINE_TEMPORARY_REGISTER 2
   %IFNUM %1
      %DEFINE t%1q x___%2q
      %DEFINE t%1d x___%2d
      %DEFINE t%1w x___%2w
      %DEFINE t%1b x___%2b
      %DEFINE t%1  x___%2
   %ELSE
      %DEFINE %1q x___%2q
      %DEFINE %1d x___%2d
      %DEFINE %1w x___%2w
      %DEFINE %1b x___%2b
      %DEFINE %1  x___%2
   %ENDIF      
   %ASSIGN _TEMPORARY_REGISTERS_MASK _TEMPORARY_REGISTERS_MASK | (1 << (%2))
   %ASSIGN _ASSIGNED_REGISTERS_MASK _ASSIGNED_REGISTERS_MASK | (1 << (%2))
%ENDMACRO   

; -----------------------------------------------------------------------------

%IF _BITS=32
   _DEFINE_REGISTER  0, UNDEF,  eax,    ax,    al
   _DEFINE_REGISTER  1, UNDEF,  ecx,    cx,    cl
   _DEFINE_REGISTER  2, UNDEF,  edx,    dx,    dl
   _DEFINE_REGISTER  3, UNDEF,  ebx,    bx,    bl
   _DEFINE_REGISTER  4, UNDEF,  esi,    si, UNDEF
   _DEFINE_REGISTER  5, UNDEF,  edi,    di, UNDEF
   _DEFINE_REGISTER  6, UNDEF,  ebp,    bp, UNDEF
   %ASSIGN _VOLATILE_REGISTERS_MASK 0x0007
   %ASSIGN _ARGUMENT_REGISTERS_MASK 0x0000
   %ASSIGN _EAX_INDEX 0
   %ASSIGN _EBX_INDEX 3
   %ASSIGN _ECX_INDEX 1
   %ASSIGN _EDX_INDEX 2
   %ASSIGN _ESI_INDEX 4
   %ASSIGN _EDI_INDEX 5
   %ASSIGN _EBP_INDEX 6
   %ASSIGN _MAX_REGISTERS 7
   _DEFINE_REGISTER sp, UNDEF,  esp, UNDEF, UNDEF
%ELSE
   %IFDEF _OS_LIN
      _DEFINE_REGISTER  0,   rdi,  edi,    di,   dil
      _DEFINE_REGISTER  1,   rsi,  esi,    si,   sil
      _DEFINE_REGISTER  2,   rdx,  edx,    dx,    dl
      _DEFINE_REGISTER  3,   rcx,  ecx,    cx,    cl
      _DEFINE_REGISTER  4,    r8,  r8d,   r8w,   r8b
      _DEFINE_REGISTER  5,    r9,  r9d,   r9w,   r9b
      _DEFINE_REGISTER  6,   rax,  eax,    ax,    al
      %ASSIGN _VOLATILE_REGISTERS_MASK 0x007F
      %ASSIGN _ARGUMENT_REGISTERS_MASK 0x003F
      %ASSIGN _EAX_INDEX 6
      %ASSIGN _EBX_INDEX 7
      %ASSIGN _ECX_INDEX 3
      %ASSIGN _EDX_INDEX 2
      %ASSIGN _ESI_INDEX 1
      %ASSIGN _EDI_INDEX 0
      %ASSIGN _EBP_INDEX 8
      %ASSIGN _2_ALIAS_INDEX 11
      %ASSIGN _3_ALIAS_INDEX 12
   %ELSE
      _DEFINE_REGISTER  0,   rcx,  ecx,    cx,    cl
      _DEFINE_REGISTER  1,   rdx,  edx,    dx,    dl
      _DEFINE_REGISTER  2,    r8,  r8d,   r8w,   r8b
      _DEFINE_REGISTER  3,    r9,  r9d,   r9w,   r9b
      _DEFINE_REGISTER  4,   rax,  eax,    ax,    al
      _DEFINE_REGISTER  5,   rdi,  edi,    di,   dil
      _DEFINE_REGISTER  6,   rsi,  esi,    si,   sil
      %ASSIGN _VOLATILE_REGISTERS_MASK 0x001F
      %ASSIGN _ARGUMENT_REGISTERS_MASK 0x000F
      %ASSIGN _EAX_INDEX 4
      %ASSIGN _EBX_INDEX 7
      %ASSIGN _ECX_INDEX 0
      %ASSIGN _EDX_INDEX 1
      %ASSIGN _ESI_INDEX 6
      %ASSIGN _EDI_INDEX 5
      %ASSIGN _EBP_INDEX 8
   %ENDIF   
   _DEFINE_REGISTER  7,   rbx,  ebx,    bx,    bl
   _DEFINE_REGISTER  8,   rbp,  ebp,    bp,   bpl
   _DEFINE_REGISTER  9,   r10, r10d,  r10w,  r10b
   _DEFINE_REGISTER 10,   r11, r11d,  r11w,  r11b
   _DEFINE_REGISTER 11,   r12, r12d,  r12w,  r12b
   _DEFINE_REGISTER 12,   r13, r13d,  r13w,  r13b
   %ASSIGN _MAX_REGISTERS 13
   _DEFINE_REGISTER sp,   rsp,  esp, UNDEF, UNDEF
%ENDIF

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
;  - whatever the method : _ARGUMENT(x) gives the address of the xth argument 
; (starting from 0) and _VARIABLE(x) gives the address of the xth aligned byte. 
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

%MACRO _DECLARE_UNUSED_TEMPORARY 1-2
   %ASSIGN %%I 0
   %REP 32
      %IF ((_ASSIGNED_REGISTERS_MASK >> %%I) & 1 == 0) && ((_ARGUMENT_REGISTERS_MASK & ((1 << %1) - 1)) >> %%I & 1 == 0)
         %IF %0 > 1
            _DEFINE_TEMPORARY_REGISTER %2, %%I
         %ENDIF
         _DEFINE_TEMPORARY_REGISTER _NUM_TEMPORARY_REGISTERS, %%I
         %ASSIGN _NUM_TEMPORARY_REGISTERS _NUM_TEMPORARY_REGISTERS + 1
         %EXITREP
      %ENDIF
      %ASSIGN %%I %%I + 1
   %ENDREP
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO _DECLARE_USED_TEMPORARY 2
   %ASSIGN %%INDEX _%1_INDEX
   _DEFINE_TEMPORARY_REGISTER %2, %%INDEX
   %ASSIGN _NUM_TEMPORARY_REGISTERS _NUM_TEMPORARY_REGISTERS + 1
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO _DECLARE_TEMPORARY 1-2
   %IF %0 = 1
      _DECLARE_UNUSED_TEMPORARY %1
   %ELIFIDN %2, xax
      _DECLARE_USED_TEMPORARY EAX, xax
   %ELIFIDN %2, xbx
      _DECLARE_USED_TEMPORARY EBX, xbx
   %ELIFIDN %2, xcx
      _DECLARE_USED_TEMPORARY ECX, xcx
   %ELIFIDN %2, xdx
      _DECLARE_USED_TEMPORARY EDX, xdx
   %ELIFIDN %2, xdi
      _DECLARE_USED_TEMPORARY EDI, xdi
   %ELIFIDN %2, xsi
      _DECLARE_USED_TEMPORARY ESI, xsi
   %ELIFIDN %2, xbp
      _DECLARE_USED_TEMPORARY EBP, xbp
   %ELSE
      _DECLARE_UNUSED_TEMPORARY %1, %2
   %ENDIF
%ENDMACRO  

; -----------------------------------------------------------------------------

%MACRO _START_PROLOGUE 0
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO _DEFINE_CONCAT 3
   %XDEFINE %1 %2%3
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO _ASSIGN_ARGUMENT_REGISTER 2-3

   %ASSIGN %%INDEX 0
   
   ; If the argument is passed by register, and if that register isn't assigned, 
   ; set %%INDEX to that register.
   ; Else, find a proper register and store its index into %%INDEX, and if the 
   ; argument was passed as a register, flag it for further copy
   
   %IF ((_ARGUMENT_REGISTERS_MASK >> %1) & 1 == 1) && ((_ASSIGNED_REGISTERS_MASK >> %1) & 1 == 0)
      %ASSIGN %%INDEX %1
   %ELSE      
      %REP _MAX_REGISTERS
         %IF ((_ASSIGNED_REGISTERS_MASK >> %%INDEX) & 1 == 0) && ((_ARGUMENT_REGISTERS_MASK >> %%INDEX) & 1 == 0)
            %EXITREP
         %ENDIF
         %ASSIGN %%INDEX %%INDEX + 1
      %ENDREP
      
      %IF (_ARGUMENT_REGISTERS_MASK >> %1) & 1 == 1
         %ASSIGN _FORCED_REGISTER_SOURCE_MASK _FORCED_REGISTER_SOURCE_MASK | (%1 << (4 * _NUM_FORCED_REGISTERS))
         %ASSIGN _FORCED_REGISTER_DESTINATION_MASK _FORCED_REGISTER_DESTINATION_MASK | (%%INDEX << (4 * _NUM_FORCED_REGISTERS))
         %ASSIGN _NUM_FORCED_REGISTERS _NUM_FORCED_REGISTERS + 1
      %ENDIF
      
   %ENDIF
   
   ; Alias the name of the argument to the free register
   
   _DEFINE_CONCAT %2, x___, %%INDEX

   ; Mark the register as used

   %ASSIGN _ASSIGNED_REGISTERS_MASK _ASSIGNED_REGISTERS_MASK | (1 << %%INDEX)

   ; Mark the register as loaded, if the argument was asked to be loaded
   ; and if the argument wasn't already in a register

   %IFIDNI %3, load
      %IF ((_ARGUMENT_REGISTERS_MASK >> %1) & 1 == 0) || ((_ASSIGNED_REGISTERS_MASK >> %1) & 1 == 0)
         %ASSIGN _LOADED_ARGUMENTS_REGISTER_MASK _LOADED_ARGUMENTS_REGISTER_MASK | (%%INDEX << (4 * _NUM_LOADED_REGISTERS))
         %ASSIGN _LOADED_ARGUMENTS_POSITION_MASK _LOADED_ARGUMENTS_POSITION_MASK | (%1 << (4 * _NUM_LOADED_REGISTERS))
         %ASSIGN _NUM_LOADED_REGISTERS _NUM_LOADED_REGISTERS + 1
      %ENDIF
   %ENDIF

%ENDMACRO   

; -----------------------------------------------------------------------------
      
%MACRO _DECLARE_ARGUMENT 1-*

   ; if we don't need the argument to be loaded into a register, and
   ; if the argument isn't already in a register, alias the argument name
   ; to its stack position (do note the subtle use of %XDEFINE)
   ; additionally : don't try to make a macro and pass [_ARGUMENT(%%K)] to it.
   ; as soon as it becomes a macro parameter, it gets entirely expanded,
   ; and so does _STACK_OFFSET, which breaks the whole purpose of _ARGUMENT
   ; if _STACK_OFFSET is incremented afterwards
   
   %IFNIDNI %2, load
      %IF (_ARGUMENT_REGISTERS_MASK >> _NUM_ARGUMENTS) & 1 == 0
         %XDEFINE %%K _NUM_ARGUMENTS
         %DEFINE %1q qword [_ARGUMENT(%%K)]
         %DEFINE %1d dword [_ARGUMENT(%%K)]
         %DEFINE %1w word [_ARGUMENT(%%K)]
         %DEFINE %1b byte [_ARGUMENT(%%K)]
         %DEFINE %1 [_ARGUMENT(%%K)]
      %ENDIF
   %ENDIF

   ; if the argument is passed by register, or if a load is requested
   ; we must assign the argument to that register. 
   ; If the register was reserved as a temporary one,
   ; we must additionnaly allocate another register, and flag it
   ; so that its value will be properly set once registers are pushed
   ; on the stack
   
   %IFIDNI %2, load
      _ASSIGN_ARGUMENT_REGISTER _NUM_ARGUMENTS, %1, load
   %ELIF (_ARGUMENT_REGISTERS_MASK >> _NUM_ARGUMENTS) & 1 == 1
      _ASSIGN_ARGUMENT_REGISTER _NUM_ARGUMENTS, %1
   %ENDIF
   
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO _SAVE_REGISTERS 0
   %ASSIGN %%I 0
   %REP _MAX_REGISTERS
      %IF (_VOLATILE_REGISTERS_MASK >> %%I) & 1 == 0
         %IF (_ASSIGNED_REGISTERS_MASK >> %%I) & 1 == 1
            _PUSH x___ %+ %%I
         %ENDIF
      %ENDIF
      %ASSIGN %%I %%I + 1
   %ENDREP
%ENDMACRO  

; -----------------------------------------------------------------------------

%MACRO _LOAD_REGISTERS 0
   %ASSIGN %%I _MAX_REGISTERS
   %REP _MAX_REGISTERS
      %ASSIGN %%I %%I - 1
      %IF (_VOLATILE_REGISTERS_MASK >> %%I) & 1 == 0
         %IF (_ASSIGNED_REGISTERS_MASK >> %%I) & 1 == 1
            _POP x___ %+ %%I
         %ENDIF
      %ENDIF
   %ENDREP
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO ARGUMENTS 0-*
   %pushd
   %ASSIGN _ASSIGNED_REGISTERS_MASK 0
   %ASSIGN _LOADED_ARGUMENTS_REGISTER_MASK 0
   %ASSIGN _LOADED_ARGUMENTS_POSITION_MASK 0
   %ASSIGN _NUM_LOADED_REGISTERS 0
   %ASSIGN _STACK_OFFSET 0
   
   _DEFINE_ARGUMENT
   %REP %0 / 2
      %ASSIGN _NUM_ARGUMENTS %1
      _DECLARE_ARGUMENT %2, load
      %ROTATE 2
   %ENDREP
   
   %ASSIGN %%I 0
   %REP _NUM_LOADED_REGISTERS
      %ASSIGN %%REGISTER (_LOADED_ARGUMENTS_REGISTER_MASK >> (4 * %%I)) & 0xF
      %ASSIGN %%POSITION (_LOADED_ARGUMENTS_POSITION_MASK >> (4 * %%I)) & 0xF
      mov x___ %+ %%REGISTER, [_ARGUMENT(%%POSITION)]
      %ASSIGN %%I %%I + 1
   %ENDREP
   
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO ENDARGUMENTS 0
   %popd
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO STACK 0-*

   ; Push a defines context on the defines stack. All further defines will be destroyed
   ; once %popd is called

   %pushd

   ; That macro defines _ARGUMENT. It is defined by default, but can be undefined if
   ; the user want to align the stack without losing a register. 

   _DEFINE_ARGUMENT

   ; Initialize some variables
   
   ; _STACK_OFFSET indicates by how much the stack pointer was moved from its original position
   ; It allows access to function arguments that are on the stack
      
   %ASSIGN _STACK_OFFSET 0
   
   ; _NUM_ARGUMENTS contains the number of arguments (registers or on the stack) for the function
   
   %ASSIGN _NUM_ARGUMENTS 0
   
   ; _NUM_TEMPORARY_REGISTERS constains the number of temporary registers requested for the function
   ; _TEMPORARY_REGISTERS_MASK indicates which registers are used as temporaries
   ; _ASSIGNED_REGISTERS_MASK indicates which registers are used
   
   %ASSIGN _NUM_TEMPORARY_REGISTERS 0
   %ASSIGN _TEMPORARY_REGISTERS_MASK 0
   %ASSIGN _ASSIGNED_REGISTERS_MASK 0
   
   ; _NUM_LOADED_REGISTERS contains the number of arguments that must be loaded from the stack into a register
   ; _LOADED_ARGUMENTS_POSITION_MASK indicates which argument is the xth loaded argument 
   ; _LOADED_ARGUMENTS_REGISTER_MASK indicates into which register the xth loaded argument will go

   %ASSIGN _NUM_LOADED_REGISTERS 0
   %ASSIGN _LOADED_ARGUMENTS_REGISTER_MASK 0
   %ASSIGN _LOADED_ARGUMENTS_POSITION_MASK 0

   ; _NUM_FORCED_REGISTERS contains the number of explicitely named temporary registers that are used to stored function arguments
   ; When that happens, the argument must be put in another register (note : we could put it on the stack, but we always have
   ; enough registers to store it in a register, because - at the moment - argument in register equals 64bits equals plenty of regs
   ; _FORCED_REGISTER_SOURCE_MASK indicates into which register the xth forced argument register was originally stored
   ; _FORCED_REGISTER_DESTINATION_MASK indicates into which register the xth forced argument will be stored
      
   %ASSIGN _NUM_FORCED_REGISTERS 0
   %ASSIGN _FORCED_REGISTER_SOURCE_MASK 0
   %ASSIGN _FORCED_REGISTER_DESTINATION_MASK 0
   
   ; _NUM_ALIASES contains the number of aliases created. Each alias must be undefined afterwards, and information for 
   ; undefining the xth alias is stored in _ALIAS_x
   
   %ASSIGN _NUM_ALIASES 0
   
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
               _DECLARE_TEMPORARY unused, xax
            %ELIFIDNI %3, xbx
               _DECLARE_TEMPORARY unused, xbx
            %ELIFIDNI %3, xcx
               _DECLARE_TEMPORARY unused, xcx
            %ELIFIDNI %3, xdx
               _DECLARE_TEMPORARY unused, xdx
            %ELIFIDNI %3, xsi
               _DECLARE_TEMPORARY unused, xsi
            %ELIFIDNI %3, xdi
               _DECLARE_TEMPORARY unused, xdi
            %ELSE
               _DECLARE_TEMPORARY unused, xbp
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
            _DECLARE_TEMPORARY %%NUM_ARGUMENTS, %1
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
               _DECLARE_ARGUMENT %1, load
            %ELSE
               _DECLARE_ARGUMENT %1
            %ENDIF
            %ASSIGN _NUM_ARGUMENTS _NUM_ARGUMENTS + 1
         %ENDIF
      %ENDIF
      %ASSIGN %%POSITION %%POSITION + 1
      %ROTATE 1
   %ENDREP
   
   ; Now that we know exactly which registers will be used, save callee-saved registers on the stack

   _SAVE_REGISTERS
   
   ; Compute the size to allocate on the stack for variables
   
   %ASSIGN %%TOTAL_VARIABLES_SIZE 0
   %ASSIGN %%POSITION 0
   %REP %0
      %IF %%POSITION >= %%START_VARIABLES + %%VARIABLES_PROLOGUE && %%POSITION < %%STOP_VARIABLES
         %IFID %1
            %IFNUM %2
               %ASSIGN %%TOTAL_VARIABLES_SIZE %%TOTAL_VARIABLES_SIZE + %2
            %ELSE
               %ASSIGN %%TOTAL_VARIABLES_SIZE %%TOTAL_VARIABLES_SIZE + _BYTES
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
            _ALLOCATE_VARIABLES %%TOTAL_VARIABLES_SIZE, %3, args, aligned
         %ELSE
            _ALLOCATE_VARIABLES %%TOTAL_VARIABLES_SIZE, xbp, args, aligned
         %ENDIF
      %ELSE
         _ALLOCATE_VARIABLES %%TOTAL_VARIABLES_SIZE
      %ENDIF
   %ELSE
      _ALLOCATE_VARIABLES %%TOTAL_VARIABLES_SIZE
   %ENDIF
   
   ; Now, create the aliases between variable addresses and variable names.
   ; Note, once again, the use of xdefine
   
   %ASSIGN %%POSITION 0
   %ASSIGN %%CURRENT_VARIABLES_SIZE 0
   %REP %0
      %IF %%POSITION >= %%START_VARIABLES + %%VARIABLES_PROLOGUE && %%POSITION < %%STOP_VARIABLES
         %IFID %1
            %XDEFINE %%CURRENT_VARIABLES_SIZE_FREEZED %%CURRENT_VARIABLES_SIZE
            %DEFINE %1 _VARIABLE(%%CURRENT_VARIABLES_SIZE_FREEZED)
            
            %IFNUM %2
               %ASSIGN %%CURRENT_VARIABLES_SIZE %%CURRENT_VARIABLES_SIZE + %2
            %ELSE
               %ASSIGN %%CURRENT_VARIABLES_SIZE %%CURRENT_VARIABLES_SIZE + _BYTES
            %ENDIF
         %ENDIF
      %ENDIF
      %ASSIGN %%POSITION %%POSITION + 1
      %ROTATE 1
   %ENDREP
   
   ; We're almost done : load arguments that needs to be loaded
      
   %ASSIGN %%I 0
   %REP _NUM_LOADED_REGISTERS
      %ASSIGN %%REGISTER (_LOADED_ARGUMENTS_REGISTER_MASK >> (4 * %%I)) & 0xF
      %ASSIGN %%POSITION (_LOADED_ARGUMENTS_POSITION_MASK >> (4 * %%I)) & 0xF
      mov x___ %+ %%REGISTER, [_ARGUMENT(%%POSITION)]
      %ASSIGN %%I %%I + 1
   %ENDREP

   ; We're really close : move arguments whose register was reserved into another register
      
   %ASSIGN %%I 0
   %REP _NUM_FORCED_REGISTERS
      %ASSIGN %%SOURCE (_FORCED_REGISTER_SOURCE_MASK >> (4 * %%I)) & 0xF
      %ASSIGN %%DESTINATION (_FORCED_REGISTER_DESTINATION_MASK >> (4 * %%I)) & 0xF
      mov x___ %+ %%DESTINATION, x___ %+ %%SOURCE
      %ASSIGN %%I %%I + 1
   %ENDREP      
   
   ; Yeah ! Finished
   
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO UNSTACK 0
   _FREE_VARIABLES
   _LOAD_REGISTERS
   
   %popd
%ENDMACRO

; -----------------------------------------------------------------------------

%MACRO RETURN 0
   UNSTACK
   ret
%ENDMACRO

; -----------------------------------------------------------------------------

%INCLUDE "xarch-simd.asm"
%INCLUDE "xarch-constants.asm"
