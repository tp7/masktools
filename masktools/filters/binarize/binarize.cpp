#include "binarize.h"
#include "../../common/simd.h"

using namespace Filtering;

typedef Byte (Operator)(Byte, Byte);

static inline Byte upper(Byte x, Byte t) { return x > t ? 0 : 255; }
static inline Byte lower(Byte x, Byte t) { return x > t ? 255 : 0; }

static inline Byte binarize_0_x(Byte x, Byte t) { return x > t ? 0 : x; }
static inline Byte binarize_t_x(Byte x, Byte t) { return x > t ? t : x; }
static inline Byte binarize_x_0(Byte x, Byte t) { return x > t ? x : 0; }
static inline Byte binarize_x_t(Byte x, Byte t) { return x > t ? x : t; }

static inline Byte binarize_t_0(Byte x, Byte t) { return x > t ? t : 0; }
static inline Byte binarize_0_t(Byte x, Byte t) { return x > t ? 0 : t; }

static inline Byte binarize_x_255(Byte x, Byte t) { return x > t ? x : 255; }
static inline Byte binarize_t_255(Byte x, Byte t) { return x > t ? t : 255; }
static inline Byte binarize_255_x(Byte x, Byte t) { return x > t ? 255 : x; }
static inline Byte binarize_255_t(Byte x, Byte t) { return x > t ? 255 : t; }


template <Operator op>
void binarize_t(Byte *pDst, ptrdiff_t nDstPitch, Byte nThreshold, int nWidth, int nHeight)
{
   for ( int j = 0; j < nHeight; j++, pDst += nDstPitch )
      for ( int i = 0; i < nWidth; i++ )
         pDst[i] = op(pDst[i], nThreshold);
}

/* SSE2 functions */
static inline __m128i upper_sse2_op(__m128i x, __m128i t, __m128i) {
    auto r = _mm_subs_epu8(x, t);
    return _mm_cmpeq_epi8(r, _mm_setzero_si128());
}

static inline __m128i lower_sse2_op(__m128i x, __m128i, __m128i t128) {
    auto r = _mm_add_epi8(x, _mm_set1_epi32(0x80808080));
    return _mm_cmpgt_epi8(r, t128);
}

static inline __m128i binarize_0_x_sse2_op(__m128i x, __m128i t, __m128i t128) {
    auto upper = upper_sse2_op(x, t, t128);
    return _mm_and_si128(upper, x);
}

static inline __m128i binarize_t_x_sse2_op(__m128i x, __m128i t, __m128i) {
    return _mm_min_epu8(t, x);
}

static inline __m128i binarize_x_0_sse2_op(__m128i x, __m128i t, __m128i t128) {
    auto lower = lower_sse2_op(x, t, t128);
    return _mm_and_si128(lower, x);
}

static inline __m128i binarize_x_t_sse2_op(__m128i x, __m128i t, __m128i) {
    return _mm_max_epu8(t, x);
}

static inline __m128i binarize_t_0_sse2_op(__m128i x, __m128i t, __m128i t128) {
    auto lower = lower_sse2_op(x, t, t128);
    return _mm_and_si128(lower, t);
}

static inline __m128i binarize_0_t_sse2_op(__m128i x, __m128i t, __m128i t128) {
    auto upper = upper_sse2_op(x, t, t128);
    return _mm_and_si128(upper, t);
}

static inline __m128i binarize_x_255_sse2_op(__m128i x, __m128i t, __m128i t128) {
    auto upper = upper_sse2_op(x, t, t128);
    return _mm_or_si128(upper, x);
}

static inline __m128i binarize_255_x_sse2_op(__m128i x, __m128i t, __m128i t128) {
    auto lower = lower_sse2_op(x, t, t128);
    return _mm_or_si128(lower, x);
}

static inline __m128i binarize_t_255_sse2_op(__m128i x, __m128i t, __m128i t128) {
    auto upper = upper_sse2_op(x, t, t128);
    return _mm_or_si128(upper, t);
}

static inline __m128i binarize_255_t_sse2_op(__m128i x, __m128i t, __m128i t128) {
    auto lower = lower_sse2_op(x, t, t128);
    return _mm_or_si128(lower, t);
}

template<decltype(simd_load_epi128) load, decltype(simd_store_epi128) store, 
    decltype(upper_sse2_op) op, decltype(upper) op_c>
void binarize_sse2_t(Byte *pDst, ptrdiff_t nDstPitch, Byte nThreshold, int nWidth, int nHeight)
{
    auto t = _mm_set1_epi32((nThreshold << 24) | (nThreshold << 16) | (nThreshold << 8) | nThreshold);
    auto t128 = _mm_add_epi8(t, _mm_set1_epi32(0x80808080));
    int wMod16 = (nWidth * 16) / 16;
    auto pDst2 = pDst;

    for ( int j = 0; j < nHeight; ++j ) {
        for ( int i = 0; i < wMod16; i+=16 ) {
            auto src = load(reinterpret_cast<const __m128i*>(pDst+i));
            auto result = op(src, t, t128);
            store(reinterpret_cast<__m128i*>(pDst+i), result);
        }
        pDst += nDstPitch;
    }

    if (nWidth > wMod16) {
        binarize_t<op_c>(pDst2 + (nWidth - wMod16), nDstPitch, nThreshold, nWidth - wMod16, nHeight);
    }
}

namespace Filtering { namespace MaskTools { namespace Filters { namespace Binarize {

Processor *upper_c = &binarize_t<upper>;
Processor *lower_c = &binarize_t<lower>;
Processor *binarize_0_x_c = &binarize_t<binarize_0_x>;
Processor *binarize_t_x_c = &binarize_t<binarize_t_x>;
Processor *binarize_x_0_c = &binarize_t<binarize_x_0>;
Processor *binarize_x_t_c = &binarize_t<binarize_x_t>;
Processor *binarize_t_0_c = &binarize_t<binarize_t_0>;
Processor *binarize_0_t_c = &binarize_t<binarize_0_t>;
Processor *binarize_x_255_c = &binarize_t<binarize_x_255>;
Processor *binarize_t_255_c = &binarize_t<binarize_t_255>;
Processor *binarize_255_x_c = &binarize_t<binarize_255_x>;
Processor *binarize_255_t_c = &binarize_t<binarize_255_t>;


Processor *upper_sse2 = &binarize_sse2_t<simd_loadu_epi128, simd_storeu_epi128, upper_sse2_op, upper>;
Processor *lower_sse2 = &binarize_sse2_t<simd_loadu_epi128, simd_storeu_epi128, lower_sse2_op, lower>;
Processor *binarize_0_x_sse2 = &binarize_sse2_t<simd_loadu_epi128, simd_storeu_epi128, binarize_0_x_sse2_op, binarize_0_x>;
Processor *binarize_t_x_sse2 = &binarize_sse2_t<simd_loadu_epi128, simd_storeu_epi128, binarize_t_x_sse2_op, binarize_t_x>;
Processor *binarize_x_0_sse2 = &binarize_sse2_t<simd_loadu_epi128, simd_storeu_epi128, binarize_x_0_sse2_op, binarize_x_0>;
Processor *binarize_x_t_sse2 = &binarize_sse2_t<simd_loadu_epi128, simd_storeu_epi128, binarize_x_t_sse2_op, binarize_x_t>;
Processor *binarize_t_0_sse2 = &binarize_sse2_t<simd_loadu_epi128, simd_storeu_epi128, binarize_t_0_sse2_op, binarize_t_0>;
Processor *binarize_0_t_sse2 = &binarize_sse2_t<simd_loadu_epi128, simd_storeu_epi128, binarize_0_t_sse2_op, binarize_0_t>;
Processor *binarize_x_255_sse2 = &binarize_sse2_t<simd_loadu_epi128, simd_storeu_epi128, binarize_x_255_sse2_op, binarize_x_255>;
Processor *binarize_t_255_sse2 = &binarize_sse2_t<simd_loadu_epi128, simd_storeu_epi128, binarize_t_255_sse2_op, binarize_t_255>;
Processor *binarize_255_x_sse2 = &binarize_sse2_t<simd_loadu_epi128, simd_storeu_epi128, binarize_255_x_sse2_op, binarize_255_x>;
Processor *binarize_255_t_sse2 = &binarize_sse2_t<simd_loadu_epi128, simd_storeu_epi128, binarize_255_t_sse2_op, binarize_255_t>;

Processor *upper_asse2 = &binarize_sse2_t<simd_load_epi128, simd_store_epi128, upper_sse2_op, upper>;
Processor *lower_asse2 = &binarize_sse2_t<simd_load_epi128, simd_store_epi128, lower_sse2_op, lower>;
Processor *binarize_0_x_asse2 = &binarize_sse2_t<simd_load_epi128, simd_store_epi128, binarize_0_x_sse2_op, binarize_0_x>;
Processor *binarize_t_x_asse2 = &binarize_sse2_t<simd_load_epi128, simd_store_epi128, binarize_t_x_sse2_op, binarize_t_x>;
Processor *binarize_x_0_asse2 = &binarize_sse2_t<simd_load_epi128, simd_store_epi128, binarize_x_0_sse2_op, binarize_x_0>;
Processor *binarize_x_t_asse2 = &binarize_sse2_t<simd_load_epi128, simd_store_epi128, binarize_x_t_sse2_op, binarize_x_t>;
Processor *binarize_t_0_asse2 = &binarize_sse2_t<simd_load_epi128, simd_store_epi128, binarize_t_0_sse2_op, binarize_t_0>;
Processor *binarize_0_t_asse2 = &binarize_sse2_t<simd_load_epi128, simd_store_epi128, binarize_0_t_sse2_op, binarize_0_t>;
Processor *binarize_x_255_asse2 = &binarize_sse2_t<simd_load_epi128, simd_store_epi128, binarize_x_255_sse2_op, binarize_x_255>;
Processor *binarize_t_255_asse2 = &binarize_sse2_t<simd_load_epi128, simd_store_epi128, binarize_t_255_sse2_op, binarize_t_255>;
Processor *binarize_255_x_asse2 = &binarize_sse2_t<simd_load_epi128, simd_store_epi128, binarize_255_x_sse2_op, binarize_255_x>;
Processor *binarize_255_t_asse2 = &binarize_sse2_t<simd_load_epi128, simd_store_epi128, binarize_255_t_sse2_op, binarize_255_t>;



} } } }