#include "makediff.h"
#include "../../../common/simd.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Support  { namespace MakeDiff {

void makediff_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight)
{
   for ( int y = 0; y < nHeight; y++, pDst += nDstPitch, pSrc += nSrcPitch )
      for ( int x = 0; x < nWidth; x++ )
         pDst[x] = clip<Byte, int>( int(pDst[x]) - pSrc[x] + 128, 0, 255 );
}


template<MemoryMode mem_mode>
static void makediff_sse2_t(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight)
{
    int wMod32 = (nWidth / 32) * 32;
    auto pDst2 = pDst;
    auto pSrc2 = pSrc;
    auto v128 = _mm_set1_epi32(0x80808080);

    for ( int j = 0; j < nHeight; ++j ) {
        for ( int i = 0; i < wMod32; i+=32 ) {
            _mm_prefetch(reinterpret_cast<const char*>(pDst)+i+128, _MM_HINT_T0);
            _mm_prefetch(reinterpret_cast<const char*>(pSrc)+i+128, _MM_HINT_T0);

            auto dst = simd_load_epi128<mem_mode>(reinterpret_cast<const __m128i*>(pDst+i));
            auto dst2 = simd_load_epi128<mem_mode>(reinterpret_cast<const __m128i*>(pDst+i+16));
            auto src = simd_load_epi128<mem_mode>(reinterpret_cast<const __m128i*>(pSrc+i));
            auto src2 = simd_load_epi128<mem_mode>(reinterpret_cast<const __m128i*>(pSrc+i+16));

            auto dstsub = _mm_sub_epi8(dst, v128);
            auto dstsub2 = _mm_sub_epi8(dst2, v128);

            auto srcsub = _mm_sub_epi8(src, v128);
            auto srcsub2 = _mm_sub_epi8(src2, v128);

            auto subbed = _mm_subs_epi8(dstsub, srcsub);
            auto subbed2 = _mm_subs_epi8(dstsub2, srcsub2);

            auto result = _mm_add_epi8(subbed, v128);
            auto result2 = _mm_add_epi8(subbed2, v128);

            simd_store_epi128<mem_mode>(reinterpret_cast<__m128i*>(pDst+i), result);
            simd_store_epi128<mem_mode>(reinterpret_cast<__m128i*>(pDst+i+16), result2);
        }
        pDst += nDstPitch;
        pSrc += nSrcPitch;
    }

    if (nWidth > wMod32) {
        makediff_c(pDst2 + wMod32, nDstPitch, pSrc2 + wMod32, nSrcPitch, nWidth - wMod32, nHeight);
    }
}

Processor *makediff_sse2 = &makediff_sse2_t<MemoryMode::SSE2_UNALIGNED>;
Processor *makediff_asse2 = &makediff_sse2_t<MemoryMode::SSE2_ALIGNED>;

} } } } }