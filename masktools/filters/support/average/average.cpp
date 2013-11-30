#include "average.h"
#include "../../../common/simd.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Support  { namespace Average {

void average_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight)
{
    for ( int y = 0; y < nHeight; y++, pDst += nDstPitch, pSrc += nSrcPitch )
        for ( int x = 0; x < nWidth; x++ )
            pDst[x] = (int(pDst[x]) + pSrc[x] + 1) >> 1;
}

template<MemoryMode mem_mode>
static void average_sse2_t(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight)
{
    int wMod16 = (nWidth / 16) * 16;
    auto pDst2 = pDst;
    auto pSrc2 = pSrc;

    for ( int j = 0; j < nHeight; ++j ) {
        for ( int i = 0; i < wMod16; i+=16 ) {
            _mm_prefetch(reinterpret_cast<const char*>(pDst)+i+256, _MM_HINT_T0);
            _mm_prefetch(reinterpret_cast<const char*>(pSrc)+i+256, _MM_HINT_T0);

            auto dst = simd_load_epi128<mem_mode>(pDst+i);
            auto src = simd_load_epi128<mem_mode>(pSrc+i);

            auto result  = _mm_avg_epu8(dst, src);

            simd_store_epi128<mem_mode>(pDst+i, result);
        }
        pDst += nDstPitch;
        pSrc += nSrcPitch;
    }

    if (nWidth > wMod16) {
        average_c(pDst2 + wMod16, nDstPitch, pSrc2 + wMod16, nSrcPitch, nWidth - wMod16, nHeight);
    }
}

Processor *average_sse2 = &average_sse2_t<MemoryMode::SSE2_UNALIGNED>;
Processor *average_asse2 = &average_sse2_t<MemoryMode::SSE2_ALIGNED>;

} } } } }