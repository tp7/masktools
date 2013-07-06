#include "average.h"
#include "../../../common/simd.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Support  { namespace Average {

void average_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight)
{
    for ( int y = 0; y < nHeight; y++, pDst += nDstPitch, pSrc += nSrcPitch )
        for ( int x = 0; x < nWidth; x++ )
            pDst[x] = (int(pDst[x]) + pSrc[x] + 1) >> 1;
}

template<decltype(simd_load_epi128) load, decltype(simd_store_epi128) store>
static FORCEINLINE void process_block_average_sse2(Byte *pDst, const Byte *pSrc)
{
    auto dst  = load(reinterpret_cast<const __m128i*>(pDst));
    auto src  = load(reinterpret_cast<const __m128i*>(pSrc));

    auto result  = _mm_avg_epu8(dst, src);

    store(reinterpret_cast<__m128i*>(pDst), result);
}

template<decltype(simd_load_epi128) load, decltype(simd_store_epi128) store>
static void average_sse2_t(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight)
{
    int wMod64 = (nWidth / 64) * 64;
    int wMod16 = (nWidth / 16) * 16;
    auto pDst2 = pDst;
    auto pSrc2 = pSrc;

    for ( int j = 0; j < nHeight; ++j ) {
        for ( int i = 0; i < wMod64; i+=64 ) {
            _mm_prefetch(reinterpret_cast<const char*>(pDst)+i+256, _MM_HINT_T0);
            _mm_prefetch(reinterpret_cast<const char*>(pSrc)+i+256, _MM_HINT_T0);

            process_block_average_sse2<load,store>(pDst+i, pSrc+i);
            process_block_average_sse2<load,store>(pDst+i+16, pSrc+i+16);
            process_block_average_sse2<load,store>(pDst+i+32, pSrc+i+32);
            process_block_average_sse2<load,store>(pDst+i+48, pSrc+i+48);
        }
        pDst += nDstPitch;
        pSrc += nSrcPitch;
    }

    if (nWidth > wMod16) {
        pDst = pDst2 + wMod64;
        pSrc = pSrc2 + wMod64;

        for ( int j = 0; j < nHeight; ++j ) {
            for ( int i = 0; i < wMod16 - wMod64; i+=16 ) {
                 process_block_average_sse2<load,store>(pDst+i, pSrc+i);
            }
            pDst += nDstPitch;
            pSrc += nSrcPitch;
        }
    }
    if (nWidth > wMod16) {
        average_c(pDst2 + wMod16, nDstPitch, pSrc2 + wMod16, nSrcPitch, nWidth - wMod16, nHeight);
    }
}

Processor *average_sse2 = &average_sse2_t<simd_loadu_epi128, simd_storeu_epi128>;
Processor *average_asse2 = &average_sse2_t<simd_load_epi128, simd_store_epi128>;

} } } } }