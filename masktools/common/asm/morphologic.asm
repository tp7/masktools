%INCLUDE "common.asm"

%MACRO MT_LOAD_UNPACK 7

   %1                      %3, pbSource + %7, nSourcePitch, m7
   %2                      %4, pbSource + %7, nSourcePitch, m7
   mova                    %5, %3
   mova                    %6, %4
   punpcklbw               %3, m6
   punpcklbw               %4, m6
   punpckhbw               %5, m6
   punpckhbw               %6, m6
   
%ENDMACRO

%MACRO MT_LOAD_UNPACK_ADD 7

   MT_LOAD_UNPACK          %{1:7}
   paddw                   %3, %4
   paddw                   %5, %6
   
%ENDMACRO

%MACRO MT_SQUARE_XXFLATE_CORE 12

   MT_LOAD_UNPACK_ADD      %1, %2, m0, m1, m2, m3, %12
   MT_LOAD_UNPACK_ADD      %3, %4, m1, m3, m4, m5, %12
   paddw                   m0, m1
   paddw                   m2, m4
   MT_LOAD_UNPACK_ADD      %5, %6, m1, m3, m4, m5, %12
   paddw                   m0, m1
   paddw                   m2, m4
   MT_LOAD_UNPACK_ADD      %7, %8, m1, m3, m4, m5, %12
   movau                   m5, [pbSource + %12 + nSourcePitch]
   paddw                   m0, m1
   paddw                   m2, m4
   psraw                   m0, 3
   psraw                   m2, 3
   mova                    m3, m5
   packuswb                m0, m2
   %11                     m5, nMaxDeviationSimd
   %9                      m0, m3
   %10                     m0, m5
   
   movau                   [pbDestination + %12], m0
   
%ENDMACRO   

%MACRO MT_SQUARE_XXPAND_CORE 12

   movau          m0, [pbSource + %12 + nSourcePitch]

   %1             m1, pbSource + %12, nSourcePitch, m7 
   %2             m2, pbSource + %12, nSourcePitch, m7
   mova           m4, m0
   
   %IFIDNI movu, mova

      %3          m0, pbSource + %12, nSourcePitch, m7, %9, m6
      %4          m1, pbSource + %12, nSourcePitch, m7, %9, m6
      %5          m2, pbSource + %12, nSourcePitch, m7, %9, m6 
      %6          m0, pbSource + %12, nSourcePitch, m7, %9, m6
      %7          m1, pbSource + %12, nSourcePitch, m7, %9, m6 
      %8          m2, pbSource + %12, nSourcePitch, m7, %9, m6

      %9          m0, m1
      %11         m4, m5
      %9          m0, m2
      
   %ELSE
   
      %3          m3, pbSource + %12, nSourcePitch, m7
      %4          m6, pbSource + %12, nSourcePitch, m7
      %9          m0, m1
      %9          m2, m3
      %5          m1, pbSource + %12, nSourcePitch, m7
      %6          m3, pbSource + %12, nSourcePitch, m7
      %9          m0, m6
      %9          m2, m1
      %7          m6, pbSource + %12, nSourcePitch, m7
      %8          m1, pbSource + %12, nSourcePitch, m7
      %9          m2, m3
      %9          m0, m6
      %9          m2, m1
      %11         m4, m5
      %9          m0, m2

   %ENDIF
   
   %10            m0, m4
   
   movau          [pbDestination + %12], m0

%ENDMACRO

%MACRO MT_BOTH_XXPAND_CORE 12

   movau          m0, [pbSource + %12 + nSourcePitch]

   %4             m1, pbSource + %12, nSourcePitch, m7 
   %5             m2, pbSource + %12, nSourcePitch, m7
   mova           m4, m0
   
   %IFIDNI movu, mova

      %2          m1, pbSource + %12, nSourcePitch, m7, %9, m6
      %7          m2, pbSource + %12, nSourcePitch, m7, %9, m6

      %9          m0, m1
      %11         m4, m5
      %9          m0, m2
      
   %ELSE
   
      %2          m3, pbSource + %12, nSourcePitch, m7
      %7          m6, pbSource + %12, nSourcePitch, m7
      %9          m0, m1
      %9          m2, m3
      %9          m0, m6
      %11         m4, m5
      %9          m0, m2

   %ENDIF
   
   %10            m0, m4
   
   movau          [pbDestination + %12], m0

%ENDMACRO

%MACRO MT_HORIZONTAL_XXPAND_CORE 12

   movau          m0, [pbSource + %12 + nSourcePitch]

   %4             m1, pbSource + %12, nSourcePitch, m7 
   %5             m2, pbSource + %12, nSourcePitch, m7
   mova           m4, m0
   %9             m0, m1
   %11            m4, m5
   %9             m0, m2
   %10            m0, m4
   
   movau          [pbDestination + %12], m0

%ENDMACRO

%MACRO MT_VERTICAL_XXPAND_CORE 12

   movau          m0, [pbSource + %12 + nSourcePitch]

   %2             m1, pbSource + %12, nSourcePitch, m7 
   %7             m2, pbSource + %12, nSourcePitch, m7
   mova           m4, m0
   %9             m0, m1
   %11            m4, m5
   %9             m0, m2
   %10            m0, m4
   
   movau          [pbDestination + %12], m0

%ENDMACRO

%DEFINE MT_EXPAND_OPERATORS      pmaxub, pminub, paddusb
%DEFINE MT_INPAND_OPERATORS      pminub, pmaxub, psubusb
%DEFINE MT_INFLATE_OPERATORS     pmaxub, pminub, paddusb
%DEFINE MT_DEFLATE_OPERATORS     pminub, pmaxub, psubusb

%MACRO MT_MORPHOLOGIC_8          12
   ASM_START_MMX
   %9                            %{1:8}, %{10:12}, 0
   ASM_STOP_MMX
%ENDMACRO

%MACRO MT_MORPHOLOGIC_64         12

   %ASSIGN offset 0
   %REP 64 / mfull
   
      %9                         %{1:8}, %{10:12}, offset
      
      %ASSIGN offset             offset + mfull
      
   %ENDREP      
   
%ENDMACRO

%MACRO MT_MORPHOLOGIC_NEXT_LINE  0
   add                           pbSource, nSourceNextRow
   add                           pbDestination, nDestinationNextRow
%ENDMACRO

%MACRO MT_MORPHOLOGIC_NEXT_8_PIXELS 0
   add                           pbSource, 8
   add                           pbDestination, 8
%ENDMACRO

%MACRO MT_MORPHOLOGIC_NEXT_64_PIXELS 0
   add                           pbSource, 64
   add                           pbDestination, 64
%ENDMACRO

%MACRO MT_MORPHOLOGIC_FUNCTION  4
%2 %+ 8_ %+ %1:
   STACK                   stack, aligned, nMaxDeviationSimd, 16, nTailingWidth, nUnrolledWidth, nSourceNextRow, nDestinationNextRow, nHeightCount,, xax, xdx,, pbDestination, *, nDestinationPitch, pbSource, *, nSourcePitch, *, nMaxDeviation, nUnused1, nUnused2, nWidth, nHeight
   ASM_XMMX                %1

   movd                    m5, nMaxDeviationd
   psplatbr                m5
   mova                    nMaxDeviationSimd, m5
   pxor                    m6, m6
  
   ASM_START_MMX
   movd                    m5, nMaxDeviationd
   psplatbr                m5
   pxor                    m6, m6
   ASM_STOP_MMX
   
   sub                     pbSource, nSourcePitch      ; positive offset needed for pitches
   
   mov                     xdx, nSourcePitch
   sub                     xdxd, nWidthd
   mov                     nSourceNextRow, xdx

   mov                     xdx, nDestinationPitch
   sub                     xdxd, nWidthd
   mov                     nDestinationNextRow, xdx

   MT_PROCESS_CONVOLUTION  %3, %4, MT_MORPHOLOGIC_NEXT_LINE, MT_MORPHOLOGIC_NEXT_8_PIXELS, MT_MORPHOLOGIC_NEXT_64_PIXELS
   
   RETURN
%ENDMACRO