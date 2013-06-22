#include "average.h"
#include "../../../common/simd.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Support  { namespace Average {

void average_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight)
{
   for ( int y = 0; y < nHeight; y++, pDst += nDstPitch, pSrc += nSrcPitch )
      for ( int x = 0; x < nWidth; x++ )
         pDst[x] = (int(pDst[x]) + pSrc[x] + 1) >> 1;
}

static inline void average_mmx(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight)
{
    for ( int j = 0; j < nHeight; ++j ) {
        for ( int i = 0; i < nWidth; i+=8 ) {
            *reinterpret_cast<__m64*>(pDst+i) = _m_pavgb(*reinterpret_cast<const __m64*>(pDst + i), *reinterpret_cast<const __m64*>(pSrc + i));
        }
        pDst += nDstPitch;
        pSrc += nSrcPitch;
    }
    _m_empty();
}

template<decltype(simd_load_epi128) load, decltype(simd_store_epi128) store>
static void average_sse2_t(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight)
{
    int wMod64 = (nWidth / 64) * 64;
    int wMod8 = (nWidth / 8) * 8;
    auto pDst2 = pDst;
    auto pSrc2 = pSrc;

    for ( int j = 0; j < nHeight; ++j ) {
        for ( int i = 0; i < wMod64; i+=64 ) {
            _mm_prefetch(reinterpret_cast<const char*>(pDst)+i+256, _MM_HINT_T0);
            _mm_prefetch(reinterpret_cast<const char*>(pSrc)+i+256, _MM_HINT_T0);

            auto dst  = load(reinterpret_cast<const __m128i*>(pDst+i));
            auto dst2 = load(reinterpret_cast<const __m128i*>(pDst+i+16));
            auto dst3 = load(reinterpret_cast<const __m128i*>(pDst+i+32));
            auto dst4 = load(reinterpret_cast<const __m128i*>(pDst+i+48));
            auto src  = load(reinterpret_cast<const __m128i*>(pSrc+i));
            auto src2 = load(reinterpret_cast<const __m128i*>(pSrc+i+16));
            auto src3 = load(reinterpret_cast<const __m128i*>(pSrc+i+32));
            auto src4 = load(reinterpret_cast<const __m128i*>(pSrc+i+48));

            auto result  = _mm_avg_epu8(dst, src);
            auto result2 = _mm_avg_epu8(dst2, src2);
            auto result3 = _mm_avg_epu8(dst3, src3);
            auto result4 = _mm_avg_epu8(dst4, src4);

            store(reinterpret_cast<__m128i*>(pDst+i), result);
            store(reinterpret_cast<__m128i*>(pDst+i+16), result2);
            store(reinterpret_cast<__m128i*>(pDst+i+32), result3);
            store(reinterpret_cast<__m128i*>(pDst+i+48), result4);
        }
        pDst += nDstPitch;
        pSrc += nSrcPitch;
    }

    if (nWidth > wMod64) {
        average_mmx(pDst2 + wMod64, nDstPitch, pSrc2 + wMod64, nSrcPitch, wMod8 - wMod64, nHeight);
    }
    if (nWidth > wMod8) {
        average_c(pDst2 + wMod8, nDstPitch, pSrc2 + wMod8, nSrcPitch, nWidth - wMod8, nHeight);
    }
}

Processor *average_sse2 = &average_sse2_t<simd_loadu_epi128, simd_storeu_epi128>;
Processor *average_asse2 = &average_sse2_t<simd_load_epi128, simd_store_epi128>;

} } } } }