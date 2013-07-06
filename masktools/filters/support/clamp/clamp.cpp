#include "clamp.h"
#include "../../../common/simd.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Support  { namespace Clamp {

void clamp_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch, const Byte *pSrc2, ptrdiff_t nSrc2Pitch, int nWidth, int nHeight, int nOvershoot, int nUndershoot)
{
   for ( int y = 0; y < nHeight; y++, pDst += nDstPitch, pSrc1 += nSrc1Pitch, pSrc2 += nSrc2Pitch )
   {
      for ( int x = 0; x < nWidth; x++ )
      {
         pDst[x] = static_cast<Byte>(pDst[x] > pSrc1[x] + nOvershoot ? pSrc1[x] + nOvershoot : pDst[x]);
         pDst[x] = static_cast<Byte>(pDst[x] < pSrc2[x] - nUndershoot ? pSrc2[x] - nUndershoot : pDst[x]);
      }
   }
}

template<decltype(simd_load_epi128) load, decltype(simd_store_epi128) store>
static void clamp_sse2_t(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch, const Byte *pSrc2, ptrdiff_t nSrc2Pitch, int nWidth, int nHeight, int nOvershoot, int nUndershoot)
{
    int wMod16 = (nWidth / 16) * 16;
    auto pDst_s = pDst;
    auto pSrc1_s = pSrc1;
    auto pSrc2_s = pSrc2;

    auto overshoot_v = simd_set8_epi32(nOvershoot);
    auto undershoot_v = simd_set8_epi32(nUndershoot);

    for ( int j = 0; j < nHeight; ++j ) {
        for ( int i = 0; i < wMod16; i+=16 ) {
            auto upper_limit = load(reinterpret_cast<const __m128i*>(pSrc1+i));
            auto lower_limit = load(reinterpret_cast<const __m128i*>(pSrc2+i));

            upper_limit = _mm_adds_epu8(upper_limit, overshoot_v);
            lower_limit = _mm_subs_epu8(lower_limit, undershoot_v);

            auto limited = load(reinterpret_cast<const __m128i*>(pDst+i));

            limited = _mm_min_epu8(limited, upper_limit);
            limited = _mm_max_epu8(limited, lower_limit);

            store(reinterpret_cast<__m128i*>(pDst+i), limited);
        }
        pDst += nDstPitch;
        pSrc1 += nSrc1Pitch;
        pSrc2 += nSrc2Pitch;
    }

    if (nWidth > wMod16) {
        clamp_c(pDst_s + wMod16, nDstPitch, pSrc1_s + wMod16, nSrc1Pitch, pSrc2_s+wMod16, nSrc2Pitch, nWidth - wMod16, nHeight, nOvershoot, nUndershoot);
    }
}

Processor *clamp_sse2 = &clamp_sse2_t<simd_loadu_epi128, simd_storeu_epi128>;
Processor *clamp_asse2 = &clamp_sse2_t<simd_load_epi128, simd_store_epi128>;

} } } } }