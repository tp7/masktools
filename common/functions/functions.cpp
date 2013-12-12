#include "functions.h"
#include <immintrin.h>

using namespace Filtering;

void Functions::memset_plane(Byte *ptr, ptrdiff_t pitch, int width, int height, Byte value)
{
    if (pitch == width) {
        memset(ptr, value, width*height);
    } else {
        for (int y = 0; y < height; ++y) {
            memset(ptr, value, width);
            ptr += pitch;
        }
    }
}

void Functions::copy_plane(Byte *pDst, ptrdiff_t dst_pitch, const Byte *pSrc, ptrdiff_t src_pitch, int width, int height)
{
    if (dst_pitch == width && src_pitch == width) {
        memcpy(pDst, pSrc, width*height);
    } else {
        for (int y = 0; y < height; ++y) {
            memcpy(pDst, pSrc, width);
            pDst += dst_pitch;
            pSrc += src_pitch;
        }
    }
}

CpuFlags Functions::get_cpu_flags()
{
    int CPUInfo[4]; //eax, ebx, ecx, edx
    CpuFlags flags = CPU_NONE;

    __cpuid(CPUInfo, 1);

    if (CPUInfo[3] & 0x00800000) flags |= CPU_MMX;
    if (CPUInfo[3] & 0x02000000) flags |= CPU_ISSE;
    if (CPUInfo[3] & 0x04000000) flags |= CPU_SSE2;
    if (CPUInfo[2] & 0x00000001) flags |= CPU_SSE3;
    if (CPUInfo[2] & 0x00000200) flags |= CPU_SSSE3;
    if (CPUInfo[2] & 0x00080000) flags |= CPU_SSE4_1;
    if (CPUInfo[2] & 0x00100000) flags |= CPU_SSE4_2;

    return flags;
}
