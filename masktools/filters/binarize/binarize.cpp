#include "binarize.h"
#include "../../common/simd.h"

using namespace Filtering;

typedef Byte (Operator)(Byte, Byte);

inline Byte upper(Byte x, Byte t) { return x > t ? 0 : 255; }
inline Byte lower(Byte x, Byte t) { return x > t ? 255 : 0; }

inline Byte binarize_0_x(Byte x, Byte t) { return x > t ? 0 : x; }
inline Byte binarize_t_x(Byte x, Byte t) { return x > t ? t : x; }
inline Byte binarize_x_0(Byte x, Byte t) { return x > t ? x : 0; }
inline Byte binarize_x_t(Byte x, Byte t) { return x > t ? x : t; }

inline Byte binarize_t_0(Byte x, Byte t) { return x > t ? t : 0; }
inline Byte binarize_0_t(Byte x, Byte t) { return x > t ? 0 : t; }

inline Byte binarize_x_255(Byte x, Byte t) { return x > t ? x : 255; }
inline Byte binarize_t_255(Byte x, Byte t) { return x > t ? t : 255; }
inline Byte binarize_255_x(Byte x, Byte t) { return x > t ? 255 : x; }
inline Byte binarize_255_t(Byte x, Byte t) { return x > t ? 255 : t; }


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
    int wMod32 = (nWidth / 32) * 32;
    auto pDst2 = pDst;

    for ( int j = 0; j < nHeight; ++j ) {
        for ( int i = 0; i < wMod32; i+=32 ) {
            _mm_prefetch(reinterpret_cast<const char*>(pDst)+i+320, _MM_HINT_T0);
            auto src = load(reinterpret_cast<const __m128i*>(pDst+i));
            auto src2 = load(reinterpret_cast<const __m128i*>(pDst+i+16));
            auto result = op(src, t, t128);
            auto result2 = op(src2, t, t128);
            store(reinterpret_cast<__m128i*>(pDst+i), result);
            store(reinterpret_cast<__m128i*>(pDst+i+16), result2);
        }
        pDst += nDstPitch;
    }

    if (nWidth > wMod32) {
        binarize_t<op_c>(pDst2 + wMod32, nDstPitch, nThreshold, nWidth - wMod32, nHeight);
    }
}

namespace Filtering { namespace MaskTools { namespace Filters { namespace Binarize {

Processor *upper_c          = &binarize_t<upper>;
Processor *lower_c          = &binarize_t<lower>;
Processor *binarize_0_x_c   = &binarize_t<binarize_0_x>;
Processor *binarize_t_x_c   = &binarize_t<binarize_t_x>;
Processor *binarize_x_0_c   = &binarize_t<binarize_x_0>;
Processor *binarize_x_t_c   = &binarize_t<binarize_x_t>;
Processor *binarize_t_0_c   = &binarize_t<binarize_t_0>;
Processor *binarize_0_t_c   = &binarize_t<binarize_0_t>;
Processor *binarize_x_255_c = &binarize_t<binarize_x_255>;
Processor *binarize_t_255_c = &binarize_t<binarize_t_255>;
Processor *binarize_255_x_c = &binarize_t<binarize_255_x>;
Processor *binarize_255_t_c = &binarize_t<binarize_255_t>;

#define DEFINE_SSE2_VERSIONS(name, load, store) \
Processor *upper_##name          = &binarize_sse2_t<load, store, upper_sse2_op, upper>; \
Processor *lower_##name          = &binarize_sse2_t<load, store, lower_sse2_op, lower>; \
Processor *binarize_0_x_##name   = &binarize_sse2_t<load, store, binarize_0_x_sse2_op, binarize_0_x>; \
Processor *binarize_t_x_##name   = &binarize_sse2_t<load, store, binarize_t_x_sse2_op, binarize_t_x>; \
Processor *binarize_x_0_##name   = &binarize_sse2_t<load, store, binarize_x_0_sse2_op, binarize_x_0>; \
Processor *binarize_x_t_##name   = &binarize_sse2_t<load, store, binarize_x_t_sse2_op, binarize_x_t>; \
Processor *binarize_t_0_##name   = &binarize_sse2_t<load, store, binarize_t_0_sse2_op, binarize_t_0>; \
Processor *binarize_0_t_##name   = &binarize_sse2_t<load, store, binarize_0_t_sse2_op, binarize_0_t>; \
Processor *binarize_x_255_##name = &binarize_sse2_t<load, store, binarize_x_255_sse2_op, binarize_x_255>; \
Processor *binarize_t_255_##name = &binarize_sse2_t<load, store, binarize_t_255_sse2_op, binarize_t_255>; \
Processor *binarize_255_x_##name = &binarize_sse2_t<load, store, binarize_255_x_sse2_op, binarize_255_x>; \
Processor *binarize_255_t_##name = &binarize_sse2_t<load, store, binarize_255_t_sse2_op, binarize_255_t>;


DEFINE_SSE2_VERSIONS(sse2, simd_loadu_epi128, simd_storeu_epi128)
DEFINE_SSE2_VERSIONS(asse2, simd_load_epi128, simd_store_epi128)




} } } }