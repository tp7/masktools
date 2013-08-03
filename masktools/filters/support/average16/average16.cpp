#include "average16.h"
#include "../../../common/simd.h"
#include "../../../common/16bit.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Support  { namespace Average16 {

void average16_stacked_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight)
{
    auto pDstLsb = pDst + nDstPitch * nHeight / 2;
    auto pSrcLsb = pSrc + nSrcPitch * nHeight / 2;

    for (int y = 0; y < nHeight / 2; y++) {
        for (int x = 0; x < nWidth; x++) {
            Word dst = read_word_stacked(pDst, pDstLsb, x);
            Word src = read_word_stacked(pSrc, pSrcLsb, x);

            Word avg = (dst + src + 1) >> 1;

            write_word_stacked(pDst, pDstLsb, x, avg);
        }
        pDst += nDstPitch;
        pDstLsb += nDstPitch;
        pSrc += nSrcPitch;
        pSrcLsb += nSrcPitch;
    }
}

void average16_interleaved_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight)
{
    for (int y = 0; y < nHeight; y++) {
        auto pDstWord = reinterpret_cast<Word*>(pDst);
        auto pSrcWord = reinterpret_cast<const Word*>(pSrc);

        for (int x = 0; x < nWidth / 2; x++) {
            pDstWord[x] = (pDstWord[x] + pSrcWord[x] + 1) >> 1;
        }
        pDst += nDstPitch;
        pSrc += nSrcPitch;
    }
}


void average16_stacked_sse2(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight)
{
    int wMod8 = (nWidth / 8) * 8;

    auto pDst2 = pDst;
    auto pSrc2 = pSrc;
    auto pDstLsb = pDst + nDstPitch * nHeight / 2;
    auto pSrcLsb = pSrc + nSrcPitch * nHeight / 2;

    auto ff = _mm_set1_epi16(0x00FF);
    auto zero = _mm_setzero_si128();

    for ( int j = 0; j < nHeight / 2; ++j ) {
        for ( int i = 0; i < wMod8; i+=8 ) {
            auto dst = read_word_stacked_simd(pDst, pDstLsb, i);
            auto src = read_word_stacked_simd(pSrc, pSrcLsb, i);

            auto result = _mm_avg_epu16(dst, src);

            write_word_stacked_simd(pDst, pDstLsb, i, result, ff, zero);
        }
        pDst += nDstPitch;
        pDstLsb += nDstPitch;
        pSrc += nSrcPitch;
        pSrcLsb += nSrcPitch;
    }
    if (nWidth > wMod8) {
        average16_stacked_c(pDst2 + wMod8, nDstPitch, pSrc2 + wMod8, nSrcPitch, nWidth - wMod8, nHeight);
    }
}

void average16_interleaved_sse2(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight)
{
    int wMod16 = (nWidth / 16) * 16;

    auto pDst2 = pDst;
    auto pSrc2 = pSrc;

    for (int j = 0; j < nHeight; ++j) {
        for (int i = 0; i < wMod16; i+=16) {
            auto dst = simd_loadu_epi128(reinterpret_cast<const __m128i*>(pDst+i));
            auto src = simd_loadu_epi128(reinterpret_cast<const __m128i*>(pSrc+i));

            auto result = _mm_avg_epu16(dst, src);

            simd_storeu_epi128(reinterpret_cast<__m128i*>(pDst+i), result);
        }
        pDst += nDstPitch;
        pSrc += nSrcPitch;
    }
    if (nWidth > wMod16) {
        average16_interleaved_c(pDst2 + wMod16, nDstPitch, pSrc2 + wMod16, nSrcPitch, nWidth - wMod16, nHeight);
    }
}



} } } } }