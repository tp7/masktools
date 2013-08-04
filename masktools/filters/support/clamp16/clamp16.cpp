#include "clamp16.h"
#include "../../../common/simd.h"
#include "../../../common/16bit.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Support  { namespace Clamp16 {

static Word MT_FORCEINLINE clamp16_core_c(Word dst, Word upperLimit, Word lowerLimit, int overshoot, int undershoot) {
    Word result = static_cast<Word>(dst > upperLimit + overshoot ? upperLimit + overshoot : dst);
    return static_cast<Word>(result < lowerLimit - undershoot ? lowerLimit - undershoot : result);
}

static __m128i MT_FORCEINLINE clamp16_core_sse2(const __m128i &dst, const __m128i &upperLimit, const __m128i &lowerLimit, const __m128i &overshoot, const __m128i &undershoot) {
    auto upper_limit_real = _mm_adds_epu16(upperLimit, overshoot);
    auto lower_limit_real = _mm_subs_epu16(lowerLimit, undershoot);

    auto limited = _mm_min_epu16(dst, upper_limit_real);
    return _mm_max_epu16(limited, lower_limit_real);
}

void clamp16_stacked_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pUpLimit, ptrdiff_t nUpLimitPitch, const Byte *pLowLimit, ptrdiff_t nLowLimitPitch, int nWidth, int nHeight, int nOvershoot, int nUndershoot)
{
   auto pDstLsb = pDst + nDstPitch * nHeight / 2;
   auto pUpLimitLsb = pUpLimit + nUpLimitPitch * nHeight / 2;
   auto pLowLimitLsb = pLowLimit + nLowLimitPitch * nHeight / 2;

   for (int y = 0; y < nHeight / 2; y++) {
       for (int x = 0; x < nWidth; x++) {
           Word dst = read_word_stacked(pDst, pDstLsb, x);
           Word upperLimit = read_word_stacked(pUpLimit, pUpLimitLsb, x);
           Word lowerLimit = read_word_stacked(pLowLimit, pLowLimitLsb, x);

           Word result = clamp16_core_c(dst, upperLimit, lowerLimit, nOvershoot, nUndershoot);

           write_word_stacked(pDst, pDstLsb, x, result);
       }
       pDst += nDstPitch;
       pDstLsb += nDstPitch;
       pUpLimit += nUpLimitPitch;
       pUpLimitLsb += nUpLimitPitch;
       pLowLimit += nLowLimitPitch;
       pLowLimitLsb += nLowLimitPitch;
   }
}

void clamp16_interleaved_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pUpLimit, ptrdiff_t nUpLimitPitch, const Byte *pLowLimit, ptrdiff_t nLowLimitPitch, int nWidth, int nHeight, int nOvershoot, int nUndershoot)
{
    for (int y = 0; y < nHeight; y++) {
        auto pDstWord = reinterpret_cast<Word*>(pDst);
        auto pUpLimitWord = reinterpret_cast<const Word*>(pUpLimit);
        auto pLowLimitWord = reinterpret_cast<const Word*>(pLowLimit);

        for (int x = 0; x < nWidth / 2; x++) {
            pDstWord[x] = clamp16_core_c(pDstWord[x], pUpLimitWord[x], pLowLimitWord[x], nOvershoot, nUndershoot);
        }
        pDst += nDstPitch;
        pUpLimit += nUpLimitPitch;
        pLowLimit += nLowLimitPitch;
    }
}

void clamp16_stacked_sse2(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pUpLimit, ptrdiff_t nUpLimitPitch, const Byte *pLowLimit, ptrdiff_t nLowLimitPitch, int nWidth, int nHeight, int nOvershoot, int nUndershoot)
{
    int wMod8 = (nWidth / 8) * 8;
    auto pDst2 = pDst;
    auto pUpLimit2 = pUpLimit;
    auto pLowLimit2 = pLowLimit;

    auto pDstLsb = pDst + nDstPitch * nHeight / 2;
    auto pUpLimitLsb = pUpLimit + nUpLimitPitch * nHeight / 2;
    auto pLowLimitLsb = pLowLimit + nLowLimitPitch * nHeight / 2;

    auto overshoot_v = _mm_set1_epi16(Word(nOvershoot));
    auto undershoot_v = _mm_set1_epi16(Word(nUndershoot));
    auto ff = _mm_set1_epi16(0x00FF);
    auto zero = _mm_setzero_si128();

    for ( int j = 0; j < nHeight / 2; ++j ) {
        for ( int i = 0; i < wMod8; i+=8 ) {
            auto upper_limit = read_word_stacked_simd(pUpLimit, pUpLimitLsb, i);
            auto lower_limit = read_word_stacked_simd(pLowLimit, pLowLimitLsb, i);
            auto limited = read_word_stacked_simd(pDst, pDstLsb, i);

            limited = clamp16_core_sse2(limited, upper_limit, lower_limit, overshoot_v, undershoot_v);

            write_word_stacked_simd(pDst, pDstLsb, i, limited, ff, zero);
        }
        pDst += nDstPitch;
        pDstLsb += nDstPitch;
        pUpLimit += nUpLimitPitch;
        pUpLimitLsb += nUpLimitPitch;
        pLowLimit += nLowLimitPitch;
        pLowLimitLsb += nLowLimitPitch;
    }

    if (nWidth > wMod8) {
        clamp16_stacked_c(pDst2 + wMod8, nDstPitch, pUpLimit2 + wMod8, nUpLimitPitch, pLowLimit2+wMod8, nLowLimitPitch, nWidth - wMod8, nHeight, nOvershoot, nUndershoot);
    }
}

void clamp16_interleaved_sse2(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pUpLimit, ptrdiff_t nUpLimitPitch, const Byte *pLowLimit, ptrdiff_t nLowLimitPitch, int nWidth, int nHeight, int nOvershoot, int nUndershoot)
{
    int wMod16 = (nWidth / 16) * 16;
    auto pDst2 = pDst;
    auto pUpLimit2 = pUpLimit;
    auto pLowLimit2 = pLowLimit;

    auto overshoot_v = _mm_set1_epi16(Word(nOvershoot));
    auto undershoot_v = _mm_set1_epi16(Word(nUndershoot));

    for ( int j = 0; j < nHeight; ++j ) {
        for ( int i = 0; i < wMod16; i+=16 ) {
            auto upper_limit = simd_loadu_epi128(reinterpret_cast<const __m128i*>(pUpLimit+i));
            auto lower_limit = simd_loadu_epi128(reinterpret_cast<const __m128i*>(pLowLimit+i));
            auto limited = simd_loadu_epi128(reinterpret_cast<const __m128i*>(pDst+i));

            limited = clamp16_core_sse2(limited, upper_limit, lower_limit, overshoot_v, undershoot_v);

            simd_storeu_epi128(reinterpret_cast<__m128i*>(pDst+i), limited);
        }
        pDst += nDstPitch;
        pUpLimit += nUpLimitPitch;
        pLowLimit += nLowLimitPitch;
    }

    if (nWidth > wMod16) {
        clamp16_interleaved_c(pDst2 + wMod16, nDstPitch, pUpLimit2 + wMod16, nUpLimitPitch, pLowLimit2+wMod16, nLowLimitPitch, nWidth - wMod16, nHeight, nOvershoot, nUndershoot);
    }
}



} } } } }