%INCLUDE "common.asm"

;=============================================================================
; Code
;=============================================================================

SECTION .text

mt_mangle Motion, sad8_isse
mt_mangle Motion, sad8_3dnow
mt_mangle Motion, sad8_sse2
mt_mangle Motion, sad8_asse2

mt_mangle Motion, motion8_mmx
mt_mangle Motion, motion8_isse
mt_mangle Motion, motion8_sse2
mt_mangle Motion, motion8_asse2

%MACRO MT_SAD_8      0

   ASM_START_MMX

   movau             m1, [pbSource + xax]
   
   %IFIDNI movau, mova
      psadbw         m1, [pbReference + xax]
   %ELSE
      movu           m2, [pbReference + xax]
      psadbw         m1, m2
   %ENDIF
      
   paddd             m0, m1                      ; will overflow with 4096x4096 + frames
   
   ASM_STOP_MMX
   
%ENDMACRO

%MACRO MT_SAD_64     0

   prefetchnta       [pbSource + xax + 192]
   prefetchnta       [pbReference + xax + 192]
   
   %ASSIGN offset 0
   %REP 64 / (4 * mfull)

      movau          m1, [pbSource + xax + offset + 0 * mfull]
      movau          m2, [pbSource + xax + offset + 1 * mfull]
      movau          m3, [pbSource + xax + offset + 2 * mfull]
      movau          m4, [pbSource + xax + offset + 3 * mfull]
   
      %IFIDNI movau, mova
      
         psadbw      m1, [pbReference + xax + offset + 0 * mfull]
         psadbw      m2, [pbReference + xax + offset + 1 * mfull]
         psadbw      m3, [pbReference + xax + offset + 2 * mfull]
         psadbw      m4, [pbReference + xax + offset + 3 * mfull]
         
      %ELSE
      
         movu        m5, [pbReference + xax + offset + 0 * mfull]
         movu        m6, [pbReference + xax + offset + 1 * mfull]
      
         psadbw      m1, m5
         psadbw      m2, m6

         movu        m5, [pbReference + xax + offset + 2 * mfull]
         movu        m6, [pbReference + xax + offset + 3 * mfull]

         psadbw      m3, m5
         psadbw      m4, m6

      %ENDIF

      paddd          m1, m2
      paddd          m3, m4
      paddd          m1, m3
      paddd          m0, m1
      
      %ASSIGN offset offset + mfull * 4
   
   %ENDREP

%ENDMACRO

%MACRO MT_SAD_ENDLINE 0
   add                  pbReference, nReferencePitch
   add                  pbSource, nSourcePitch
%ENDMACRO

%MACRO MT_SAD_COMMON  1
sad8_ %+ %1:
   STACK                xax, xdx,, pbSource, *, nSourcePitch, pbReference, *, nReferencePitch, nWidth, *, nHeight, *
   ASM_XMMX             %1

   pxor                 m0, m0
   pxor                 mm0, mm0
   
   MT_UNROLL_INPLACE_WIDTH  MT_SAD_64, MT_SAD_8, MT_SAD_ENDLINE
   
   %IF ASM_HAS_XMM_REGISTERS
   
      movhlps           m1, m0
      paddd             m0, m1
      movd              xaxd, m0
      movd              xdxd, mm0
      add               xaxd, edxd
      
   %ELSE
      
      movd              xaxd, m0
      
   %ENDIF
      
   RETURN
%ENDMACRO

MT_SAD_COMMON  isse
MT_SAD_COMMON  3dnow
MT_SAD_COMMON  sse2
MT_SAD_COMMON  asse2
   
%MACRO MT_MOTION_CORE            1
   movau                         m0, [pbReference + xax + %1]
   movau                         m1, [pbDestination + xax + %1]
   mova                          m2, m0
   psubusb                       m0, m1
   psubusb                       m1, m2
   paddb                         m0, m1

   mova                          m1, m0
   psubb                         m1, m5
   mova                          m3, m1
   pcmpgtb                       m1, m7
   pcmpgtb                       m3, m6
   pand                          m0, m1
   por                           m0, m3
   
   movau                         [pbDestination + xax + %1], m0
%ENDMACRO

%MACRO MT_MOTION_8               0
   ASM_START_MMX
   MT_MOTION_CORE                0
   ASM_STOP_MMX
%ENDMACRO

%MACRO MT_MOTION_64              0

   %ASSIGN offset 0
   %REP 64 / mfull
   
      MT_MOTION_CORE             offset
      
      %ASSIGN offset             offset + mfull
   
   %ENDREP
   
%ENDMACRO

%MACRO MT_MOTION_NEXT_LINE 0
   add                           pbReference, nReferencePitch
   add                           pbDestination, nDestinationPitch
%ENDMACRO

%MACRO MT_MOTION_FUNCTION   1
motion8_ %+ %1:
   STACK                xax, xdx,, pbDestination, *, nDestinationPitch, pbReference, *, nReferencePitch, nLowThreshold, nHighThreshold, nWidth, *, nHeight, *
   ASM_XMMX             %1

   movd                 m6, nHighThreshold
   movd                 m7, nLowThreshold
   mova                 m5, [ASM_BYTE_0x80]
   psplatbr             m6
   psplatbr             m7
   psubb                m6, m5
   psubb                m7, m5

   ASM_START_MMX
   
   movd                 m6, nHighThreshold
   movd                 m7, nLowThreshold
   mova                 m5, [ASM_BYTE_0x80]
   psplatbr             m6
   psplatbr             m7
   psubb                m6, m5
   psubb                m7, m5
   
   ASM_STOP_MMX
      
   MT_UNROLL_INPLACE_WIDTH  MT_MOTION_64, MT_MOTION_8, MT_MOTION_NEXT_LINE
   
   RETURN
%ENDMACRO

MT_MOTION_FUNCTION mmx
MT_MOTION_FUNCTION isse
MT_MOTION_FUNCTION sse2
MT_MOTION_FUNCTION asse2
