#include "logic16.h"
#include "../../common/simd.h"
#include "../../common/16bit.h"

using namespace Filtering;

static MT_FORCEINLINE Word add_c(Word a, Word b) { return clip<Word, int>(a + (int)b); }
static MT_FORCEINLINE Word sub_c(Word a, Word b) { return clip<Word, int>(a - (int)b); }
static MT_FORCEINLINE Word nop_c(Word a, Word b) { UNUSED(b); return a; }

static MT_FORCEINLINE Word and_c(Word a, Word b, Word th1, Word th2) { UNUSED(th1); UNUSED(th2); return a & b; }
static MT_FORCEINLINE Word or_c(Word a, Word b, Word th1, Word th2) { UNUSED(th1); UNUSED(th2); return a | b; }
static MT_FORCEINLINE Word andn_c(Word a, Word b, Word th1, Word th2) { UNUSED(th1); UNUSED(th2); return a & ~b; }
static MT_FORCEINLINE Word xor_c(Word a, Word b, Word th1, Word th2) { UNUSED(th1); UNUSED(th2); return a ^ b; }

template <decltype(add_c) opa, decltype(add_c) opb>
static MT_FORCEINLINE Word min_t(Word a, Word b, Word th1, Word th2) { 
    return min<Word>(opa(a, th1), opb(b, th2)); 
}

template <decltype(add_c) opa, decltype(add_c) opb>
static MT_FORCEINLINE Word max_t(Word a, Word b, Word th1, Word th2) { 
    return max<Word>(opa(a, th1), opb(b, th2)); 
}

template <decltype(and_c) op>
void logic16_stacked_t(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight, Word nThresholdDestination, Word nThresholdSource)
{
    auto pDstLsb = pDst + nDstPitch * nHeight / 2;
    auto pSrcLsb = pSrc + nSrcPitch * nHeight / 2;

    for (int y = 0; y < nHeight / 2; y++) {
        for (int x = 0; x < nWidth; x++) {
            Word dst = read_word_stacked(pDst, pDstLsb, x);
            Word src = read_word_stacked(pSrc, pSrcLsb, x);

            Word avg = op(dst, src, nThresholdDestination, nThresholdSource);

            write_word_stacked(pDst, pDstLsb, x, avg);
        }
        pDst += nDstPitch;
        pDstLsb += nDstPitch;
        pSrc += nSrcPitch;
        pSrcLsb += nSrcPitch;
    }
}

template <decltype(and_c) op>
void logic16_interleaved_t(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight, Word nThresholdDestination, Word nThresholdSource)
{
    for (int y = 0; y < nHeight; y++) {
        auto pDstWord = reinterpret_cast<Word*>(pDst);
        auto pSrcWord = reinterpret_cast<const Word*>(pSrc);

        for (int x = 0; x < nWidth / 2; x++) {
            pDstWord[x] = op(pDstWord[x], pSrcWord[x], nThresholdDestination, nThresholdSource);
        }
        pDst += nDstPitch;
        pSrc += nSrcPitch;
    }
}

/* sse2 */

static MT_FORCEINLINE __m128i add_sse2(__m128i a, __m128i b) { return _mm_adds_epu16(a, b); }
static MT_FORCEINLINE __m128i sub_sse2(__m128i a, __m128i b) { return _mm_subs_epu16(a, b); }
static MT_FORCEINLINE __m128i nop_sse2(__m128i a, __m128i) { return a; }

static MT_FORCEINLINE __m128i and_sse2(const __m128i &a, const __m128i &b, const __m128i&, const __m128i&) { 
    return _mm_and_si128(a, b); 
}

static MT_FORCEINLINE __m128i or_sse2(const __m128i &a, const __m128i &b, const __m128i&, const __m128i&) { 
    return _mm_or_si128(a, b); 
}

static MT_FORCEINLINE __m128i andn_sse2(const __m128i &a, const __m128i &b, const __m128i&, const __m128i&) { 
    return _mm_andnot_si128(a, b); 
}

static MT_FORCEINLINE __m128i xor_sse2(const __m128i &a, const __m128i &b, const __m128i&, const __m128i&) { 
    return _mm_xor_si128(a, b); 
}

template <decltype(add_sse2) opa, decltype(add_sse2) opb>
static MT_FORCEINLINE __m128i min_t_sse2(const __m128i &a, const __m128i &b, const __m128i& th1, const __m128i& th2) { 
    return _mm_min_epu16(opa(a, th1), opb(b, th2));
}

template <decltype(add_sse2) opa, decltype(add_sse2) opb>
static MT_FORCEINLINE __m128i max_t_sse2(const __m128i &a, const __m128i &b, const __m128i& th1, const __m128i& th2) { 
    return _mm_max_epu16(opa(a, th1), opb(b, th2));
}


template<decltype(and_sse2) op, decltype(and_c) op_c>
    void logic16_stacked_t_sse2(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight, Word nThresholdDestination, Word nThresholdSource)
{
    int wMod8 = (nWidth / 8) * 8;
    auto pDst2 = pDst;
    auto pSrc2 = pSrc;
    auto pDstLsb = pDst + nDstPitch * nHeight / 2;
    auto pSrcLsb = pSrc + nSrcPitch * nHeight / 2;

    auto tDest = _mm_set1_epi16(nThresholdDestination);
    auto tSource = _mm_set1_epi16(nThresholdSource);
    auto ff = _mm_set1_epi16(0x00FF);
    auto zero = _mm_setzero_si128();

    for ( int j = 0; j < nHeight / 2; ++j ) {
        for ( int i = 0; i < wMod8; i+=8 ) {
            auto dst = read_word_stacked_simd(pDst, pDstLsb, i);
            auto src = read_word_stacked_simd(pSrc, pSrcLsb, i);

            auto result = op(dst, src, tDest, tSource);

            write_word_stacked_simd(pDst, pDstLsb, i, result, ff, zero);
        }
        pDst += nDstPitch;
        pDstLsb += nDstPitch;
        pSrc += nSrcPitch;
        pSrcLsb += nSrcPitch;
    }
    if (nWidth > wMod8) {
        logic16_stacked_t<op_c>(pDst2 + wMod8, nDstPitch, pSrc2 + wMod8, nSrcPitch, nWidth - wMod8, nHeight, nThresholdDestination, nThresholdSource);
    }
}

template<decltype(and_sse2) op, decltype(and_c) op_c>
void logic16_interleaved_t_sse2(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight, Word nThresholdDestination, Word nThresholdSource)
{
    int wMod16 = (nWidth / 16) * 16;
    auto pDst2 = pDst;
    auto pSrc2 = pSrc;
    auto tDest = _mm_set1_epi16(nThresholdDestination);
    auto tSource = _mm_set1_epi16(nThresholdSource);

    for (int j = 0; j < nHeight; ++j) {
        for (int i = 0; i < wMod16; i+=16) {
            auto dst = simd_loadu_epi128(reinterpret_cast<const __m128i*>(pDst+i));
            auto src = simd_loadu_epi128(reinterpret_cast<const __m128i*>(pSrc+i));

            auto result = op(dst, src, tDest, tSource);

            simd_storeu_epi128(reinterpret_cast<__m128i*>(pDst+i), result);
        }
        pDst += nDstPitch;
        pSrc += nSrcPitch;
    }
    if (nWidth > wMod16) {
        logic16_interleaved_t<op_c>(pDst2 + wMod16, nDstPitch, pSrc2 + wMod16, nSrcPitch, nWidth - wMod16, nHeight, nThresholdDestination, nThresholdSource);
    }
}


namespace Filtering { namespace MaskTools { namespace Filters { namespace Logic16 {

Processor *and_stacked_c      = &logic16_stacked_t<and_c>;
Processor *or_stacked_c       = &logic16_stacked_t<or_c>;
Processor *andn_stacked_c     = &logic16_stacked_t<andn_c>;
Processor *xor_stacked_c      = &logic16_stacked_t<xor_c>;

Processor *and_interleaved_c  = &logic16_interleaved_t<and_c>;
Processor *or_interleaved_c   = &logic16_interleaved_t<or_c>;
Processor *andn_interleaved_c = &logic16_interleaved_t<andn_c>;
Processor *xor_interleaved_c  = &logic16_interleaved_t<xor_c>;


#define DEFINE_SILLY_C_MODE(mode, layout) \
    Processor *mode##_##layout##_c         = &logic16_##layout##_t<mode##_t<nop_c, nop_c>>;   \
    Processor *mode##sub_##layout##_c      = &logic16_##layout##_t<mode##_t<nop_c, sub_c>>;   \
    Processor *mode##add_##layout##_c      = &logic16_##layout##_t<mode##_t<nop_c, add_c>>;   \
    Processor *sub##mode##_##layout##_c    = &logic16_##layout##_t<mode##_t<sub_c, nop_c>>;   \
    Processor *sub##mode##sub_##layout##_c = &logic16_##layout##_t<mode##_t<sub_c, sub_c>>;   \
    Processor *sub##mode##add_##layout##_c = &logic16_##layout##_t<mode##_t<sub_c, add_c>>;   \
    Processor *add##mode##_##layout##_c    = &logic16_##layout##_t<mode##_t<add_c, nop_c>>;   \
    Processor *add##mode##sub_##layout##_c = &logic16_##layout##_t<mode##_t<add_c, sub_c>>;   \
    Processor *add##mode##add_##layout##_c = &logic16_##layout##_t<mode##_t<add_c, add_c>>;

DEFINE_SILLY_C_MODE(min, stacked);
DEFINE_SILLY_C_MODE(min, interleaved);
DEFINE_SILLY_C_MODE(max, stacked);
DEFINE_SILLY_C_MODE(max, interleaved);


Processor *and_stacked_sse2      = &logic16_stacked_t_sse2<and_sse2, and_c>;
Processor *or_stacked_sse2       = &logic16_stacked_t_sse2<or_sse2, or_c>;
Processor *andn_stacked_sse2     = &logic16_stacked_t_sse2<andn_sse2, andn_c>;
Processor *xor_stacked_sse2      = &logic16_stacked_t_sse2<xor_sse2, xor_c>;

Processor *and_interleaved_sse2  = &logic16_interleaved_t_sse2<and_sse2, and_c>;
Processor *or_interleaved_sse2   = &logic16_interleaved_t_sse2<or_sse2, or_c>;
Processor *andn_interleaved_sse2 = &logic16_interleaved_t_sse2<andn_sse2, andn_c>;
Processor *xor_interleaved_sse2  = &logic16_interleaved_t_sse2<xor_sse2, xor_c>;

#define DEFINE_SILLY_SSE2_VERSIONS(mode, layout) \
    Processor *mode##_##layout##_sse2         = &logic16_##layout##_t_sse2<mode##_t_sse2<nop_sse2, nop_sse2>, mode##_t<nop_c, nop_c>>;   \
    Processor *mode##sub_##layout##_sse2      = &logic16_##layout##_t_sse2<mode##_t_sse2<nop_sse2, sub_sse2>, mode##_t<nop_c, sub_c>>;   \
    Processor *mode##add_##layout##_sse2      = &logic16_##layout##_t_sse2<mode##_t_sse2<nop_sse2, add_sse2>, mode##_t<nop_c, add_c>>;   \
    Processor *sub##mode##_##layout##_sse2    = &logic16_##layout##_t_sse2<mode##_t_sse2<sub_sse2, nop_sse2>, mode##_t<sub_c, nop_c>>;   \
    Processor *sub##mode##sub_##layout##_sse2 = &logic16_##layout##_t_sse2<mode##_t_sse2<sub_sse2, sub_sse2>, mode##_t<sub_c, sub_c>>;   \
    Processor *sub##mode##add_##layout##_sse2 = &logic16_##layout##_t_sse2<mode##_t_sse2<sub_sse2, add_sse2>, mode##_t<sub_c, add_c>>;   \
    Processor *add##mode##_##layout##_sse2    = &logic16_##layout##_t_sse2<mode##_t_sse2<add_sse2, nop_sse2>, mode##_t<add_c, nop_c>>;   \
    Processor *add##mode##sub_##layout##_sse2 = &logic16_##layout##_t_sse2<mode##_t_sse2<add_sse2, sub_sse2>, mode##_t<add_c, sub_c>>;   \
    Processor *add##mode##add_##layout##_sse2 = &logic16_##layout##_t_sse2<mode##_t_sse2<add_sse2, add_sse2>, mode##_t<add_c, add_c>>;

DEFINE_SILLY_SSE2_VERSIONS(min, stacked)
DEFINE_SILLY_SSE2_VERSIONS(max, stacked)
DEFINE_SILLY_SSE2_VERSIONS(min, interleaved)
DEFINE_SILLY_SSE2_VERSIONS(max, interleaved)

} } } }