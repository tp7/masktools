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

static inline void clamp_mmx(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch, const Byte *pSrc2, ptrdiff_t nSrc2Pitch, int nWidth, int nHeight, int nOvershoot, int nUndershoot)
{
    auto overshoot_v = _mm_set1_pi32((nOvershoot << 24) | (nOvershoot << 16) | (nOvershoot << 8) | nOvershoot);
    auto undershoot_v = _mm_set1_pi32((nUndershoot << 24) | (nUndershoot << 16) | (nUndershoot << 8) | nUndershoot);

    for ( int j = 0; j < nHeight; ++j ) {
        for ( int i = 0; i < nWidth; i+=8 ) {
            auto upper_limit = *reinterpret_cast<const __m64*>(pSrc1 + i);
            auto lower_limit = *reinterpret_cast<const __m64*>(pSrc2 + i);
            upper_limit = _mm_adds_pu8(upper_limit, overshoot_v);
            lower_limit = _mm_subs_pu8(lower_limit, undershoot_v);
            auto limited = *reinterpret_cast<const __m64*>(pDst + i);
            limited = _mm_min_pu8(limited, upper_limit);
            limited = _mm_max_pu8(limited, lower_limit);

            *reinterpret_cast<__m64*>(pDst+i) = limited;
        }
        pDst += nDstPitch;
        pSrc1 += nSrc1Pitch;
        pSrc2 += nSrc2Pitch;
    }
    _m_empty();
}

template<decltype(simd_load_epi128) load, decltype(simd_store_epi128) store>
static void clamp_sse2_t(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch, const Byte *pSrc2, ptrdiff_t nSrc2Pitch, int nWidth, int nHeight, int nOvershoot, int nUndershoot)
{
    int wMod32 = (nWidth / 32) * 32;
    int wMod8 = (nWidth / 8) * 8;
    auto pDst_s = pDst;
    auto pSrc1_s = pSrc1;
    auto pSrc2_s = pSrc2;

    auto overshoot_v = simd_set8_epi32(nOvershoot);
    auto undershoot_v = simd_set8_epi32(nUndershoot);

    for ( int j = 0; j < nHeight; ++j ) {
        for ( int i = 0; i < wMod32; i+=32 ) {
            auto upper_limit1 = load(reinterpret_cast<const __m128i*>(pSrc1+i));
            auto upper_limit2 = load(reinterpret_cast<const __m128i*>(pSrc1+i+16));
            auto lower_limit1 = load(reinterpret_cast<const __m128i*>(pSrc2+i));
            auto lower_limit2 = load(reinterpret_cast<const __m128i*>(pSrc2+i+16));

            upper_limit1 = _mm_adds_epu8(upper_limit1, overshoot_v);
            upper_limit2 = _mm_adds_epu8(upper_limit2, overshoot_v);

            lower_limit1 = _mm_subs_epu8(lower_limit1, undershoot_v);
            lower_limit2 = _mm_subs_epu8(lower_limit2, undershoot_v);

            auto limited1 = load(reinterpret_cast<const __m128i*>(pDst+i));
            auto limited2 = load(reinterpret_cast<const __m128i*>(pDst+i+16));

            limited1 = _mm_min_epu8(limited1, upper_limit1);
            limited2 = _mm_min_epu8(limited2, upper_limit2);

            limited1 = _mm_max_epu8(limited1, lower_limit1);
            limited2 = _mm_max_epu8(limited2, lower_limit2);

            store(reinterpret_cast<__m128i*>(pDst+i), limited1);
            store(reinterpret_cast<__m128i*>(pDst+i+16), limited2);
        }
        pDst += nDstPitch;
        pSrc1 += nSrc1Pitch;
        pSrc2 += nSrc2Pitch;
    }

    if (nWidth > wMod32) {
        clamp_mmx(pDst_s + wMod32, nDstPitch, pSrc1_s + wMod32, nSrc1Pitch, pSrc2_s+wMod32, nSrc2Pitch, wMod8 - wMod32, nHeight, nOvershoot, nUndershoot);
    }
    if (nWidth > wMod8) {
        clamp_c(pDst_s + wMod8, nDstPitch, pSrc1_s + wMod8, nSrc1Pitch, pSrc2_s+wMod8, nSrc2Pitch, nWidth - wMod8, nHeight, nOvershoot, nUndershoot);
    }
}

Processor *clamp_sse2 = &clamp_sse2_t<simd_loadu_epi128, simd_storeu_epi128>;
Processor *clamp_asse2 = &clamp_sse2_t<simd_load_epi128, simd_store_epi128>;

} } } } }