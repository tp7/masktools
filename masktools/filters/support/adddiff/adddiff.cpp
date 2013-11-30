#include "adddiff.h"
#include "../../../common/simd.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Support  { namespace AddDiff {

void adddiff_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight)
{
   for ( int y = 0; y < nHeight; y++, pDst += nDstPitch, pSrc += nSrcPitch )
      for ( int x = 0; x < nWidth; x++ )
         pDst[x] = clip<Byte, int>( int(pDst[x]) + pSrc[x] - 128, 0, 255 );
}


template<MemoryMode mem_mode>
static void adddiff_sse2_t(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight)
{
    int wMod32 = (nWidth / 32) * 32;
    auto pDst2 = pDst;
    auto pSrc2 = pSrc;
    auto v128 = _mm_set1_epi32(0x80808080);

    for ( int j = 0; j < nHeight; ++j ) {
        for ( int i = 0; i < wMod32; i+=32 ) {
            _mm_prefetch(reinterpret_cast<const char*>(pDst)+i+128, _MM_HINT_T0);
            _mm_prefetch(reinterpret_cast<const char*>(pSrc)+i+128, _MM_HINT_T0);

            auto dst = simd_load_epi128<mem_mode>(pDst+i);
            auto dst2 = simd_load_epi128<mem_mode>(pDst+i+16);
            auto src = simd_load_epi128<mem_mode>(pSrc+i);
            auto src2 = simd_load_epi128<mem_mode>(pSrc+i+16);

            auto dstsub = _mm_sub_epi8(dst, v128);
            auto dstsub2 = _mm_sub_epi8(dst2, v128);

            auto srcsub = _mm_sub_epi8(src, v128);
            auto srcsub2 = _mm_sub_epi8(src2, v128);

            auto added = _mm_adds_epi8(dstsub, srcsub);
            auto added2 = _mm_adds_epi8(dstsub2, srcsub2);

            auto result = _mm_add_epi8(added, v128);
            auto result2 = _mm_add_epi8(added2, v128);

            simd_store_epi128<mem_mode>(pDst+i, result);
            simd_store_epi128<mem_mode>(pDst+i+16, result2);
        }
        pDst += nDstPitch;
        pSrc += nSrcPitch;
    }

    if (nWidth > wMod32) {
        adddiff_c(pDst2 + wMod32, nDstPitch, pSrc2 + wMod32, nSrcPitch, nWidth - wMod32, nHeight);
    }
}

Processor *adddiff_sse2 = &adddiff_sse2_t<MemoryMode::SSE2_UNALIGNED>;
Processor *adddiff_asse2 = &adddiff_sse2_t<MemoryMode::SSE2_ALIGNED>;

} } } } }