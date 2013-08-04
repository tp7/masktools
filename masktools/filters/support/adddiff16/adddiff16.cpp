#include "adddiff16.h"
#include "../../../common/simd.h"
#include "../../../common/16bit.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Support  { namespace AddDiff16 {

static MT_FORCEINLINE Word adddiff16_core_c(Word dst, Word src) {
    return clip<Word, int>(int(dst) + src - (65536 / 2), 0, 65535);
}

static MT_FORCEINLINE __m128i adddiff16_core_sse2(const __m128i &dst, const __m128i &src, const __m128i &halfrange) {
    auto dstval = _mm_sub_epi16(dst, halfrange);
    auto srcval = _mm_sub_epi16(src, halfrange);
    auto result = _mm_adds_epi16(dstval, srcval);
    return _mm_add_epi16(result, halfrange);
}


void adddiff16_stacked_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight)
{
   auto pDstLsb = pDst + nDstPitch * nHeight / 2;
   auto pSrcLsb = pSrc + nSrcPitch * nHeight / 2;

   for (int y = 0; y < nHeight / 2; y++) {
       for (int x = 0; x < nWidth; x++) {
           Word dst = read_word_stacked(pDst, pDstLsb, x);
           Word src = read_word_stacked(pSrc, pSrcLsb, x);

           Word result = adddiff16_core_c(dst, src);

           write_word_stacked(pDst, pDstLsb, x, result);
       }
       pDst += nDstPitch;
       pDstLsb += nDstPitch;
       pSrc += nSrcPitch;
       pSrcLsb += nSrcPitch;
   }
}

void adddiff16_interleaved_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight)
{
    for (int y = 0; y < nHeight; y++) {
        auto pDstWord = reinterpret_cast<Word*>(pDst);
        auto pSrcWord = reinterpret_cast<const Word*>(pSrc);

        for (int x = 0; x < nWidth / 2; x++) {
            pDstWord[x] = adddiff16_core_c(pDstWord[x], pSrcWord[x]);
        }
        pDst += nDstPitch;
        pSrc += nSrcPitch;
    }
}

#pragma warning(disable: 4309)

void adddiff16_stacked_sse2(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight)
{
    int wMod8 = (nWidth / 8) * 8;
    auto pDst2 = pDst;
    auto pSrc2 = pSrc;

    auto pDstLsb = pDst + nDstPitch * nHeight / 2;
    auto pSrcLsb = pSrc + nSrcPitch * nHeight / 2;

    auto ff = _mm_set1_epi16(0x00FF);
    auto zero = _mm_setzero_si128();
    auto halfrange = _mm_set1_epi16(0x8000);

    for ( int j = 0; j < nHeight / 2; ++j ) {
        for ( int i = 0; i < wMod8; i+=8 ) {
            auto dst = read_word_stacked_simd(pDst, pDstLsb, i);
            auto src = read_word_stacked_simd(pSrc, pSrcLsb, i);

            auto result = adddiff16_core_sse2(dst, src, halfrange);

            write_word_stacked_simd(pDst, pDstLsb, i, result, ff, zero);
        }
        pDst += nDstPitch;
        pDstLsb += nDstPitch;
        pSrc += nSrcPitch;
        pSrcLsb += nSrcPitch;
    }

    if (nWidth > wMod8) {
        adddiff16_stacked_c(pDst2 + wMod8, nDstPitch, pSrc2 + wMod8, nSrcPitch, nWidth - wMod8, nHeight);
    }
}

void adddiff16_interleaved_sse2(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight)
{
    int wMod16 = (nWidth / 16) * 16;

    auto pDst2 = pDst;
    auto pSrc2 = pSrc;
    auto halfrange = _mm_set1_epi16(0x8000);

    for (int j = 0; j < nHeight; ++j) {
        for (int i = 0; i < wMod16; i+=16) {
            auto dst = simd_loadu_epi128(reinterpret_cast<const __m128i*>(pDst+i));
            auto src = simd_loadu_epi128(reinterpret_cast<const __m128i*>(pSrc+i));

            auto result = adddiff16_core_sse2(dst, src, halfrange);

            simd_storeu_epi128(reinterpret_cast<__m128i*>(pDst+i), result);
        }
        pDst += nDstPitch;
        pSrc += nSrcPitch;
    }
    if (nWidth > wMod16) {
        adddiff16_interleaved_c(pDst2 + wMod16, nDstPitch, pSrc2 + wMod16, nSrcPitch, nWidth - wMod16, nHeight);
    }
}


} } } } }