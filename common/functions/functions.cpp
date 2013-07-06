#include "functions.h"
#include <immintrin.h>

using namespace Filtering;

void Functions::memset_c(Byte *pPlane, ptrdiff_t nPitch, int nWidth, int nHeight, Byte value)
{
    if (nPitch == nWidth) {
        memset(pPlane, value, nWidth*nHeight);
    } else {
        for (int y = 0; y < nHeight; ++y) {
            memset(pPlane, value, nWidth);
            pPlane += nPitch;
        }
    }
}

void Functions::copy_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch,
                       int nWidth, int nHeight)
{
    if (nDstPitch == nWidth && nSrcPitch == nWidth) {
        memcpy(pDst, pSrc, nWidth*nHeight);
    } else {
        for (int y = 0; y < nHeight; ++y) {
            memcpy(pDst, pSrc, nWidth);
            pDst += nDstPitch;
            pSrc += nSrcPitch;
        }
    }
}

CpuFlags Functions::get_cpu_flags()
{
   int CPUInfo[4]; //eax, ebx, ecx, edx
   CpuFlags flags = CPU_NONE;

   __cpuid(CPUInfo, 1);

   if ( CPUInfo[3] & 0x00800000 ) flags |= CPU_MMX;
   if ( CPUInfo[3] & 0x02000000 ) flags |= CPU_ISSE;
   if ( CPUInfo[3] & 0x04000000 ) flags |= CPU_SSE2;
   if ( CPUInfo[2] & 0x00000001 ) flags |= CPU_SSE3;
   if ( CPUInfo[2] & 0x00000200 ) flags |= CPU_SSSE3;
   if ( CPUInfo[2] & 0x00080000 ) flags |= CPU_SSE4_1;
   if ( CPUInfo[2] & 0x00100000 ) flags |= CPU_SSE4_2;

   return flags;
} 
