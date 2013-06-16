%INCLUDE "common.asm"

;=============================================================================
; Code
;=============================================================================

SECTION .text

   mt_mangle Edge, sobel8_mmx
   mt_mangle Edge, sobel8_sse2
   mt_mangle Edge, roberts8_mmx
   mt_mangle Edge, roberts8_sse2
   mt_mangle Edge, laplace8_mmx
   mt_mangle Edge, laplace8_sse2
   mt_mangle Edge, morpho8_isse
   mt_mangle Edge, morpho8_sse2
   mt_mangle Edge, convolution8_mmx
   mt_mangle Edge, convolution8_sse2
   mt_mangle Edge, prewitt8_isse
   mt_mangle Edge, prewitt8_sse2
   mt_mangle Edge, prewitt8_ssse3
   mt_mangle Edge, half_prewitt8_isse
   mt_mangle Edge, half_prewitt8_sse2
   mt_mangle Edge, half_prewitt8_ssse3

%MACRO MT_EDGE_THRESHOLDING 4

   packuswb                %2, %3                            ; abs values
   mova                    %3, %2
   psubb                   %3, [ASM_BYTE_0x80]
   mova                    %4, %3
   pcmpgtb                 %3, nLowThresholdSimd
   pcmpgtb                 %4, nHighThresholdSimd
   pand                    %2, %3
   por                     %2, %4
   
   movau                   [%1], %2
   
%ENDMACRO   

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

%MACRO MT_LOAD_UNPACK_SUB 7

   MT_LOAD_UNPACK          %{1:7}
   psubw                   %3, %4
   psubw                   %5, %6
   
%ENDMACRO

%MACRO MT_ABS_SUB_SHIFT 7

   mova                    %5, %1
   mova                    %6, %2
   psubusw                 %1, %3
   psubusw                 %2, %4
   psubusw                 %3, %5
   psubusw                 %4, %6
   paddw                   %1, %3
   paddw                   %2, %4
   psraw                   %1, %7
   psraw                   %2, %7
   
%ENDMACRO   
   
%MACRO MT_SOBEL_CORE       5
   
   MT_LOAD_UNPACK_ADD      %1, %2, m0, m1, m2, m3, %5
   MT_LOAD_UNPACK_ADD      %3, %4, m1, m3, m4, m5, %5
   MT_ABS_SUB_SHIFT        m0, m2, m1, m4, m3, m5, 1
   
   MT_EDGE_THRESHOLDING    pbDestination + %5, m0, m2, m1
   
%ENDMACRO   

%MACRO MT_ROBERTS_CORE     3

   movau                   m0, [pbSource + %3 + nSourcePitch]
   mova                    m1, m0
   punpcklbw               m0, m6
   punpckhbw               m1, m6
   paddw                   m0, m0
   paddw                   m1, m1
   MT_LOAD_UNPACK_ADD      %1, %2, m2, m3, m4, m5, %3
   MT_ABS_SUB_SHIFT        m0, m1, m2, m4, m3, m5, 1
   
   MT_EDGE_THRESHOLDING    pbDestination + %3, m0, m1, m2

%ENDMACRO

%MACRO MT_EDGE_THRESHOLDING_BYTE 4

   mova                    %3, %2
   psubb                   %2, [ASM_BYTE_0x80]
   mova                    %4, %2
   pcmpgtb                 %2, nLowThresholdSimd
   pcmpgtb                 %4, nHighThresholdSimd
   pand                    %3, %2
   por                     %3, %4
   
   movau                   [%1], %3
   
%ENDMACRO

%MACRO MT_MORPHO_CORE      9

   movau                   m0, [pbSource + %9 + nSourcePitch]
   %1                      m1, pbSource + %9, nSourcePitch, m7
   %2                      m2, pbSource + %9, nSourcePitch, m7
   mova                    m6, m0
   %3                      m3, pbSource + %9, nSourcePitch, m7
   pminub                  m0, m1
   pmaxub                  m6, m1
   %4                      m4, pbSource + %9, nSourcePitch, m7
   pminub                  m0, m2
   pmaxub                  m6, m2
   %5                      m5, pbSource + %9, nSourcePitch, m7
   pminub                  m0, m3
   pmaxub                  m6, m3
   %6                      m1, pbSource + %9, nSourcePitch, m7
   pminub                  m0, m4
   pmaxub                  m6, m4
   %7                      m2, pbSource + %9, nSourcePitch, m7
   pminub                  m0, m5
   pmaxub                  m6, m5
   %8                      m3, pbSource + %9, nSourcePitch, m7
   pminub                  m0, m1
   pmaxub                  m6, m1
   pminub                  m0, m2
   pmaxub                  m6, m2
   pminub                  m0, m3
   pmaxub                  m6, m3
   
   psubb                   m6, m0
   
   MT_EDGE_THRESHOLDING_BYTE pbDestination + %9, m6, m0, m1
   
%ENDMACRO   

%MACRO MT_LAPLACE_CORE     9

   MT_LOAD_UNPACK_ADD      %1, %2, m0, m1, m2, m3, %9
   MT_LOAD_UNPACK_ADD      %3, %4, m1, m3, m4, m5, %9
   paddw                   m0, m1
   paddw                   m2, m4
   MT_LOAD_UNPACK_ADD      %5, %6, m1, m3, m4, m5, %9
   paddw                   m0, m1
   paddw                   m2, m4
   MT_LOAD_UNPACK_ADD      %7, %8, m1, m3, m4, m5, %9
   paddw                   m0, m1
   paddw                   m2, m4
   movau                   m1, [pbSource + %9 + nSourcePitch]
   mova                    m4, m1
   punpcklbw               m1, m6
   punpckhbw               m4, m6
   psllw                   m1, 3
   psllw                   m4, 3
   
   MT_ABS_SUB_SHIFT        m0, m2, m1, m4, m3, m5, 3
   
   MT_EDGE_THRESHOLDING    pbDestination + %9, m0, m2, m1

%ENDMACRO

%MACRO MT_CONVOLUTION_CORE 9

   MT_LOAD_UNPACK          %1, %2, m0, m2, m1, m3, %9
   pmullw                  m0, pnCoefficientsArray(0 * 16)
   pmullw                  m1, pnCoefficientsArray(0 * 16)
   pmullw                  m2, pnCoefficientsArray(1 * 16)
   pmullw                  m3, pnCoefficientsArray(1 * 16)
   paddw                   m0, m2
   paddw                   m1, m3
   MT_LOAD_UNPACK          %3, %4, m2, m4, m3, m5, %9
   pmullw                  m2, pnCoefficientsArray(2 * 16)
   pmullw                  m3, pnCoefficientsArray(2 * 16)
   pmullw                  m4, pnCoefficientsArray(3 * 16)
   pmullw                  m5, pnCoefficientsArray(3 * 16)
   paddw                   m0, m2
   paddw                   m1, m3
   paddw                   m0, m4
   paddw                   m1, m5
   MT_LOAD_UNPACK          %5, %6, m2, m4, m3, m5, %9
   pmullw                  m2, pnCoefficientsArray(5 * 16)
   pmullw                  m3, pnCoefficientsArray(5 * 16)
   pmullw                  m4, pnCoefficientsArray(6 * 16)
   pmullw                  m5, pnCoefficientsArray(6 * 16)
   paddw                   m0, m2
   paddw                   m1, m3
   paddw                   m0, m4
   paddw                   m1, m5
   MT_LOAD_UNPACK          %7, %8, m2, m4, m3, m5, %9
   pmullw                  m2, pnCoefficientsArray(7 * 16)
   pmullw                  m3, pnCoefficientsArray(7 * 16)
   pmullw                  m4, pnCoefficientsArray(8 * 16)
   pmullw                  m5, pnCoefficientsArray(8 * 16)
   paddw                   m0, m2
   paddw                   m1, m3
   movau                   m2, [pbSource + %9 + nSourcePitch]
   paddw                   m0, m4
   paddw                   m1, m5
   mova                    m3, m2
   punpcklbw               m2, m6
   punpckhbw               m3, m6
   pmullw                  m2, pnCoefficientsArray(4 * 16)
   pmullw                  m3, pnCoefficientsArray(4 * 16)
   paddw                   m0, m2
   paddw                   m1, m3
   mova                    m2, m0
   mova                    m3, m1
   mova                    m4, nShiftSimd
   psraw                   m0, 15
   psraw                   m1, 15
   pxor                    m2, m0
   pxor                    m3, m1
   psubw                   m2, m0
   psubw                   m3, m1
   psrlw                   m2, m4
   psrlw                   m3, m4
   
   MT_EDGE_THRESHOLDING    pbDestination + %9, m2, m3, m0

%ENDMACRO

%MACRO MT_PACKED_ABS       4

   %IF ASM_HAS_SSSE3
   
      pabsw                %1, %1
      pabsw                %2, %2
      packuswb             %1, %2
   
   %ELSE
   
      pxor                 %3, %3
      pxor                 %4, %4
      psubw                %3, %1
      psubw                %4, %2
      packuswb             %1, %2
      packuswb             %3, %4
      pmaxub               %1, %3
   
   %ENDIF

%ENDMACRO

%MACRO MT_PREWITT_CORE     9

   pxor                    m6, m6

   MT_LOAD_UNPACK_SUB      %2, %7, m0, m2, m1, m3, %9
   MT_LOAD_UNPACK_SUB      %1, %8, m2, m4, m3, m5, %9
   
   mova                    pnCoefficientsArray(0 * 16), m0
   mova                    pnCoefficientsArray(1 * 16), m1
   mova                    pnCoefficientsArray(2 * 16), m2
   mova                    pnCoefficientsArray(3 * 16), m3
   
   paddw                   m0, m2
   paddw                   m1, m3

   mova                    pnCoefficientsArray(4 * 16), m0
   mova                    pnCoefficientsArray(5 * 16), m1
   
   MT_LOAD_UNPACK_SUB      %4, %5, m0, m2, m1, m3, %9
   MT_LOAD_UNPACK_SUB      %6, %3, m2, m4, m3, m5, %9

   mova                    pnCoefficientsArray(6 * 16), m0
   mova                    pnCoefficientsArray(7 * 16), m1
   
   paddw                   m0, m2
   paddw                   m1, m3
   
   mova                    m4, m0
   mova                    m5, m1
   
   psubw                   m0, pnCoefficientsArray(0 * 16)
   psubw                   m1, pnCoefficientsArray(1 * 16)
   
   paddw                   m4, pnCoefficientsArray(2 * 16)
   paddw                   m5, pnCoefficientsArray(3 * 16)
   
   MT_PACKED_ABS           m0, m1, m6, m7
   MT_PACKED_ABS           m4, m5, m6, m7
   
   mova                    m1, pnCoefficientsArray(4 * 16)
   mova                    m5, pnCoefficientsArray(5 * 16)
   
   psubw                   m2, m1
   psubw                   m3, m5
   
   paddw                   m1, pnCoefficientsArray(6 * 16)
   paddw                   m5, pnCoefficientsArray(7 * 16)

   MT_PACKED_ABS           m2, m3, m6, m7
   MT_PACKED_ABS           m1, m5, m6, m7
   
   pmaxub                  m0, m4
   pmaxub                  m2, m1
   pmaxub                  m0, m2
   
   MT_EDGE_THRESHOLDING_BYTE pbDestination + %9, m0, m1, m2

%ENDMACRO

%MACRO MT_HPREWITT_CORE    9

   MT_LOAD_UNPACK_SUB      %2, %7, m0, m2, m1, m3, %9
   MT_LOAD_UNPACK_SUB      %1, %6, m2, m4, m3, m5, %9
   
   psllw                   m0, 1
   psllw                   m1, 1
   paddw                   m0, m2
   paddw                   m1, m3

   MT_LOAD_UNPACK_SUB      %3, %8, m2, m4, m3, m5, %9
   paddw                   m0, m2
   paddw                   m1, m3
   
   mova                    pnCoefficientsArray(0 * 16), m0
   mova                    pnCoefficientsArray(1 * 16), m1
   
   MT_LOAD_UNPACK_SUB      %4, %5, m0, m2, m1, m3, %9
   MT_LOAD_UNPACK_SUB      %1, %3, m2, m4, m3, m5, %9
   
   psllw                   m0, 1
   psllw                   m1, 1
   paddw                   m0, m2
   paddw                   m1, m3

   MT_LOAD_UNPACK_SUB      %6, %8, m2, m4, m3, m5, %9
   paddw                   m0, m2
   paddw                   m1, m3
   
   mova                    m2, pnCoefficientsArray(0 * 16)
   mova                    m3, pnCoefficientsArray(1 * 16)
   
   MT_PACKED_ABS           m0, m1, m4, m5
   MT_PACKED_ABS           m2, m3, m4, m5
   
   pmaxub                  m0, m2

   MT_EDGE_THRESHOLDING_BYTE pbDestination + %9, m0, m1, m2

%ENDMACRO

%MACRO MT_SOBEL_8          8
   ASM_START_MMX
   MT_SOBEL_CORE           %2, %4, %5, %7, 0
   ASM_STOP_MMX
%ENDMACRO

%MACRO MT_ROBERTS_8        8
   ASM_START_MMX
   MT_ROBERTS_CORE         %5, %7, 0
   ASM_STOP_MMX
%ENDMACRO

%MACRO MT_LAPLACE_8        8
   ASM_START_MMX
   MT_LAPLACE_CORE         %{1:8}, 0
   ASM_STOP_MMX
%ENDMACRO

%MACRO MT_MORPHO_8         8
   ASM_START_MMX
   MT_MORPHO_CORE          %{1:8}, 0
   ASM_STOP_MMX
%ENDMACRO

%MACRO MT_CONVOLUTION_8    8
   ASM_START_MMX
   MT_CONVOLUTION_CORE     %{1:8}, 0
   ASM_STOP_MMX
%ENDMACRO

%MACRO MT_PREWITT_8        8
   ASM_START_MMX
   MT_PREWITT_CORE         %{1:8}, 0
   ASM_STOP_MMX
%ENDMACRO

%MACRO MT_HPREWITT_8       8
   ASM_START_MMX
   MT_HPREWITT_CORE        %{1:8}, 0
   ASM_STOP_MMX
%ENDMACRO

%MACRO MT_SOBEL_64         8
   
   %ASSIGN offset 0
   %REP 64 / mfull
   
      MT_SOBEL_CORE        %2, %4, %5, %7, offset
   
      %ASSIGN offset       offset + mfull
   
   %ENDREP
   
%ENDMACRO   

%MACRO MT_ROBERTS_64       8
   
   %ASSIGN offset 0
   %REP 64 / mfull
   
      MT_ROBERTS_CORE      %5, %7, offset
   
      %ASSIGN offset       offset + mfull
   
   %ENDREP
   
%ENDMACRO   

%MACRO MT_LAPLACE_64       8
   
   %ASSIGN offset 0
   %REP 64 / mfull
   
      MT_LAPLACE_CORE      %{1:8}, offset
   
      %ASSIGN offset       offset + mfull
   
   %ENDREP
   
%ENDMACRO   

%MACRO MT_MORPHO_64        8
   
   %ASSIGN offset 0
   %REP 64 / mfull
   
      MT_MORPHO_CORE       %{1:8}, offset
   
      %ASSIGN offset       offset + mfull
   
   %ENDREP
   
%ENDMACRO   

%MACRO MT_CONVOLUTION_64   8
   
   %ASSIGN offset 0
   %REP 64 / mfull
   
      MT_CONVOLUTION_CORE  %{1:8}, offset
   
      %ASSIGN offset       offset + mfull
   
   %ENDREP
   
%ENDMACRO   

%MACRO MT_PREWITT_64       8
   
   %ASSIGN offset 0
   %REP 64 / mfull
   
      MT_PREWITT_CORE      %{1:8}, offset
   
      %ASSIGN offset       offset + mfull
   
   %ENDREP
   
%ENDMACRO   

%MACRO MT_HPREWITT_64      8
   
   %ASSIGN offset 0
   %REP 64 / mfull
   
      MT_HPREWITT_CORE     %{1:8}, offset
   
      %ASSIGN offset       offset + mfull
   
   %ENDREP
   
%ENDMACRO   


%MACRO MT_EDGE_NEXT_LINE         0
   add                           pbSource, nSourceNextRow
   add                           pbDestination, nDestinationNextRow
%ENDMACRO

%MACRO MT_EDGE_NEXT_8_PIXELS     0
   add                           pbSource, 8
   add                           pbDestination, 8
%ENDMACRO

%MACRO MT_EDGE_NEXT_64_PIXELS    0
   add                           pbSource, 64
   add                           pbDestination, 64
%ENDMACRO

%MACRO MT_EDGE_FUNCTION    3
%2 %+ 8_ %+ %1:
   STACK                   stack, aligned, nLowThresholdSimd, 16, nHighThresholdSimd, 16, nCoefficientsArray, 16 * 9, nShiftSimd, 16, nTailingWidth, nUnrolledWidth, nSourceNextRow, nDestinationNextRow, nHeightCount,, xax, xdx,, pbDestination, *, nDestinationPitch, pbSource, *, nSourcePitch, *, pswMatrix, nLowThreshold, nHighThreshold, nWidth, nHeight
   ASM_XMMX                %1

   %IFIDNI %2, convolution   
      mov                  xax, pswMatrix

      pinsrw               m0, [xax + 0 * 2], 0
      pinsrw               m1, [xax + 1 * 2], 0
      pinsrw               m2, [xax + 2 * 2], 0
      pinsrw               m3, [xax + 3 * 2], 0
      psplatwr             m0
      psplatwr             m1
      psplatwr             m2
      psplatwr             m3
      mova                 pnCoefficientsArray(0 * 16), m0
      mova                 pnCoefficientsArray(1 * 16), m1
      mova                 pnCoefficientsArray(2 * 16), m2
      mova                 pnCoefficientsArray(3 * 16), m3
      pinsrw               m0, [xax + 4 * 2], 0
      pinsrw               m1, [xax + 5 * 2], 0
      pinsrw               m2, [xax + 6 * 2], 0
      pinsrw               m3, [xax + 7 * 2], 0
      pinsrw               m4, [xax + 8 * 2], 0
      psplatwr             m0
      psplatwr             m1
      psplatwr             m2
      psplatwr             m3
      psplatwr             m4
      mova                 pnCoefficientsArray(4 * 16), m0
      mova                 pnCoefficientsArray(5 * 16), m1
      mova                 pnCoefficientsArray(6 * 16), m2
      mova                 pnCoefficientsArray(7 * 16), m3
      mova                 pnCoefficientsArray(8 * 16), m4
      movzx                xax, word [xax + 9 * 2]
      bsf                  xdxd, xaxd
      movd                 m0, xdxd
      mova                 nShiftSimd, m0

   %ENDIF   
   
   movd                    m5, nLowThresholdd
   movd                    m6, nHighThresholdd
   psplatbr                m5
   psplatbr                m6
   psubb                   m5, [ASM_BYTE_0x80]
   psubb                   m6, [ASM_BYTE_0x80]
   mova                    nLowThresholdSimd, m5
   mova                    nHighThresholdSimd, m6
   pxor                    m6, m6
   pxor                    mm6, mm6
   
   sub                     pbSource, nSourcePitch           ; positive offset needed for pitches
   
   mov                     xdx, nSourcePitch
   sub                     xdxd, nWidthd
   mov                     nSourceNextRow, xdx

   mov                     xdx, nDestinationPitch
   sub                     xdxd, nWidthd
   mov                     nDestinationNextRow, xdx

   MT_PROCESS_CONVOLUTION  MT_ %+ %3 %+ _64, MT_ %+ %3 %+ _8, MT_EDGE_NEXT_LINE, MT_EDGE_NEXT_8_PIXELS, MT_EDGE_NEXT_64_PIXELS
   
   RETURN
%ENDMACRO

MT_EDGE_FUNCTION        mmx, sobel, SOBEL
MT_EDGE_FUNCTION        sse2, sobel, SOBEL
   
MT_EDGE_FUNCTION        mmx, roberts, ROBERTS
MT_EDGE_FUNCTION        sse2, roberts, ROBERTS
   
MT_EDGE_FUNCTION        mmx, laplace, LAPLACE
MT_EDGE_FUNCTION        sse2, laplace, LAPLACE

MT_EDGE_FUNCTION        isse, morpho, MORPHO
MT_EDGE_FUNCTION        sse2, morpho, MORPHO
   
MT_EDGE_FUNCTION        mmx, convolution, CONVOLUTION
MT_EDGE_FUNCTION        sse2, convolution, CONVOLUTION

MT_EDGE_FUNCTION        isse, prewitt, PREWITT
MT_EDGE_FUNCTION        sse2, prewitt, PREWITT
MT_EDGE_FUNCTION        ssse3, prewitt, PREWITT

MT_EDGE_FUNCTION        isse, half_prewitt, HPREWITT
MT_EDGE_FUNCTION        sse2, half_prewitt, HPREWITT
MT_EDGE_FUNCTION        ssse3, half_prewitt, HPREWITT
