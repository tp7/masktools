#include "binarize16.h"
#include "../../common/simd.h"
#include "../../common/16bit.h"

using namespace Filtering;

typedef Word (Operator)(Word, Word);

inline Word binarize_upper(Word x, Word t) { return x > t ? 0 : 65535; }
inline Word binarize_lower(Word x, Word t) { return x > t ? 65535 : 0; }

inline Word binarize_0_x(Word x, Word t) { return x > t ? 0 : x; }
inline Word binarize_t_x(Word x, Word t) { return x > t ? t : x; }
inline Word binarize_x_0(Word x, Word t) { return x > t ? x : 0; }
inline Word binarize_x_t(Word x, Word t) { return x > t ? x : t; }

inline Word binarize_t_0(Word x, Word t) { return x > t ? t : 0; }
inline Word binarize_0_t(Word x, Word t) { return x > t ? 0 : t; }

inline Word binarize_x_255(Word x, Word t) { return x > t ? x : 65535; }
inline Word binarize_t_255(Word x, Word t) { return x > t ? t : 65535; }
inline Word binarize_255_x(Word x, Word t) { return x > t ? 65535 : x; }
inline Word binarize_255_t(Word x, Word t) { return x > t ? 65535 : t; }


template <Operator op>
void binarize_stacked_t(Byte *pDst, ptrdiff_t nDstPitch, Word nThreshold, int nWidth, int nHeight)
{
    auto pLsb = pDst + nDstPitch * nHeight / 2;

    for ( int y = 0; y < nHeight / 2; y++ )
    {
        for ( int x = 0; x < nWidth; x++ ) {
            Word result = op(read_word_stacked(pDst, pLsb, x), nThreshold);
            write_word_stacked(pDst, pLsb, x, result);
        }
        pDst += nDstPitch;
        pLsb += nDstPitch;
    }
}

template <Operator op>
void binarize_interleaved_t(Byte *pDst, ptrdiff_t nDstPitch, Word nThreshold, int nWidth, int nHeight)
{
    for ( int y = 0; y < nHeight; y++ )
    {
        auto pDst16 = reinterpret_cast<Word*>(pDst);
        for ( int x = 0; x < nWidth / 2; x+=1 )
            pDst16[x] = op(pDst16[x], nThreshold);
        pDst += nDstPitch;
    }
}

/* SSE2 functions */
static inline __m128i binarize_upper_sse2_op(__m128i x, __m128i t, __m128i) {
    auto r = _mm_subs_epu16(x, t);
    return _mm_cmpeq_epi16(r, _mm_setzero_si128());
}

static inline __m128i binarize_lower_sse2_op(__m128i x, __m128i t, __m128i halfrange) {
    auto r = _mm_add_epi16(x, t);
    return _mm_cmpgt_epi16(r, halfrange);
}

static inline __m128i binarize_0_x_sse2_op(__m128i x, __m128i t, __m128i halfrange) {
    auto upper = binarize_upper_sse2_op(x, t, halfrange);
    return _mm_and_si128(upper, x);
}

static inline __m128i binarize_t_x_sse2_op(__m128i x, __m128i t, __m128i) {
    return _mm_min_epu16(t, x);
}

static inline __m128i binarize_x_0_sse2_op(__m128i x, __m128i t, __m128i halfrange) {
    auto lower = binarize_lower_sse2_op(x, t, halfrange);
    return _mm_and_si128(lower, x);
}

static inline __m128i binarize_x_t_sse2_op(__m128i x, __m128i t, __m128i) {
    return _mm_max_epu16(t, x);
}

static inline __m128i binarize_t_0_sse2_op(__m128i x, __m128i t, __m128i halfrange) {
    auto lower = binarize_lower_sse2_op(x, t, halfrange);
    return _mm_and_si128(lower, t);
}

static inline __m128i binarize_0_t_sse2_op(__m128i x, __m128i t, __m128i halfrange) {
    auto upper = binarize_upper_sse2_op(x, t, halfrange);
    return _mm_and_si128(upper, t);
}

static inline __m128i binarize_x_255_sse2_op(__m128i x, __m128i t, __m128i halfrange) {
    auto upper = binarize_upper_sse2_op(x, t, halfrange);
    return _mm_or_si128(upper, x);
}

static inline __m128i binarize_255_x_sse2_op(__m128i x, __m128i t, __m128i halfrange) {
    auto lower = binarize_lower_sse2_op(x, t, halfrange);
    return _mm_or_si128(lower, x);
}

static inline __m128i binarize_t_255_sse2_op(__m128i x, __m128i t, __m128i halfrange) {
    auto upper = binarize_upper_sse2_op(x, t, halfrange);
    return _mm_or_si128(upper, t);
}

static inline __m128i binarize_255_t_sse2_op(__m128i x, __m128i t, __m128i halfrange) {
    auto lower = binarize_lower_sse2_op(x, t, halfrange);
    return _mm_or_si128(lower, t);
}

#pragma warning(disable: 4309)
template<decltype(binarize_upper_sse2_op) op, decltype(binarize_upper) op_c>
void binarize_sse2_stacked_t(Byte *pDst, ptrdiff_t nDstPitch, Word nThreshold, int nWidth, int nHeight)
{
    int wMod8 = (nWidth / 8) * 8;
    auto pDst2 = pDst;
    auto pDstLsb = pDst + nDstPitch * nHeight / 2;

    auto t = _mm_set1_epi16(Word(nThreshold));
    auto halfrange = _mm_add_epi16(t, _mm_set1_epi16(0x8000));
    auto ff = _mm_set1_epi16(0x00FF);
    auto zero = _mm_setzero_si128();

    for ( int j = 0; j < nHeight / 2; ++j ) {
        for ( int i = 0; i < wMod8; i+=8 ) {
            auto src =  read_word_stacked_simd(pDst, pDstLsb, i);
            auto result = op(src, t, halfrange);

            write_word_stacked_simd(pDst, pDstLsb, i, result, ff, zero);
        }
        pDst += nDstPitch;
                pDstLsb += nDstPitch;
    }

    if (nWidth > wMod8) {
        binarize_stacked_t<op_c>(pDst2 + wMod8, nDstPitch, nThreshold, nWidth - wMod8, nHeight);
    }
}

template<decltype(binarize_upper_sse2_op) op, decltype(binarize_upper) op_c>
void binarize_sse2_interleaved_t(Byte *pDst, ptrdiff_t nDstPitch, Word nThreshold, int nWidth, int nHeight)
{
    int wMod16 = (nWidth / 16) * 16;
    auto pDst2 = pDst;
    auto t = _mm_set1_epi16(Word(nThreshold));
    auto halfrange = _mm_add_epi16(t, _mm_set1_epi16(0x8000));

    for (int j = 0; j < nHeight; ++j) {
        for (int i = 0; i < wMod16; i+=16) {
            auto src = simd_loadu_epi128(reinterpret_cast<const __m128i*>(pDst+i));

            auto result = op(src, t, halfrange);

            simd_storeu_epi128(reinterpret_cast<__m128i*>(pDst+i), result);
        }
        pDst += nDstPitch;
    }
    if (nWidth > wMod16) {
        binarize_interleaved_t<op_c>(pDst2 + wMod16, nDstPitch, nThreshold, nWidth - wMod16, nHeight);
    }
}

namespace Filtering { namespace MaskTools { namespace Filters { namespace Binarize16 {

#define DEFINE_PROCESSORS(layout) \
Processor *binarize_upper_##layout##_c = &binarize_##layout##_t<binarize_upper>;  \
Processor *binarize_lower_##layout##_c = &binarize_##layout##_t<binarize_lower>;  \
Processor *binarize_0_x_##layout##_c   = &binarize_##layout##_t<binarize_0_x>;    \
Processor *binarize_t_x_##layout##_c   = &binarize_##layout##_t<binarize_t_x>;    \
Processor *binarize_x_0_##layout##_c   = &binarize_##layout##_t<binarize_x_0>;    \
Processor *binarize_x_t_##layout##_c   = &binarize_##layout##_t<binarize_x_t>;    \
Processor *binarize_t_0_##layout##_c   = &binarize_##layout##_t<binarize_t_0>;    \
Processor *binarize_0_t_##layout##_c   = &binarize_##layout##_t<binarize_0_t>;    \
Processor *binarize_x_255_##layout##_c = &binarize_##layout##_t<binarize_x_255>;  \
Processor *binarize_t_255_##layout##_c = &binarize_##layout##_t<binarize_t_255>;  \
Processor *binarize_255_x_##layout##_c = &binarize_##layout##_t<binarize_255_x>;  \
Processor *binarize_255_t_##layout##_c = &binarize_##layout##_t<binarize_255_t>;  \
    \
Processor *binarize_upper_##layout##_sse2 = &binarize_sse2_##layout##_t<binarize_upper_sse2_op, binarize_upper>; \
Processor *binarize_lower_##layout##_sse2 = &binarize_sse2_##layout##_t<binarize_lower_sse2_op, binarize_lower>; \
Processor *binarize_0_x_##layout##_sse2   = &binarize_sse2_##layout##_t<binarize_0_x_sse2_op,   binarize_0_x>;     \
Processor *binarize_t_x_##layout##_sse2   = &binarize_sse2_##layout##_t<binarize_t_x_sse2_op,   binarize_t_x>;     \
Processor *binarize_x_0_##layout##_sse2   = &binarize_sse2_##layout##_t<binarize_x_0_sse2_op,   binarize_x_0>;     \
Processor *binarize_x_t_##layout##_sse2   = &binarize_sse2_##layout##_t<binarize_x_t_sse2_op,   binarize_x_t>;     \
Processor *binarize_t_0_##layout##_sse2   = &binarize_sse2_##layout##_t<binarize_t_0_sse2_op,   binarize_t_0>;     \
Processor *binarize_0_t_##layout##_sse2   = &binarize_sse2_##layout##_t<binarize_0_t_sse2_op,   binarize_0_t>;     \
Processor *binarize_x_255_##layout##_sse2 = &binarize_sse2_##layout##_t<binarize_x_255_sse2_op, binarize_x_255>; \
Processor *binarize_t_255_##layout##_sse2 = &binarize_sse2_##layout##_t<binarize_t_255_sse2_op, binarize_t_255>; \
Processor *binarize_255_x_##layout##_sse2 = &binarize_sse2_##layout##_t<binarize_255_x_sse2_op, binarize_255_x>; \
Processor *binarize_255_t_##layout##_sse2 = &binarize_sse2_##layout##_t<binarize_255_t_sse2_op, binarize_255_t>;


DEFINE_PROCESSORS(stacked)
DEFINE_PROCESSORS(interleaved)


} } } }