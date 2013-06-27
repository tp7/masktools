#include "inflate.h"
#include "../../../filters/morphologic/functions.h"
#include "../../../common/simd.h"

using namespace Filtering;

 enum class Border {
    Left,
    Right,
    None
};

static inline Byte meanMax(Byte a1, Byte a2, Byte a3, Byte a4, Byte a5, Byte a6, Byte a7, Byte a8, Byte a9)
{
   int nSum = 0;
   nSum += a1 + a2 + a3 + a4 + a6 + a7 + a8 + a9;
   nSum >>= 3;
   return static_cast<Byte>(nSum > a5 ? nSum : a5);
}

static inline Byte meanMaxThresholded(Byte a1, Byte a2, Byte a3, Byte a4, Byte a5, Byte a6, Byte a7, Byte a8, Byte a9, int nMaxDeviation)
{
   int nMeanMax = meanMax(a1, a2, a3, a4, a5, a6, a7, a8, a9);
   if ( nMeanMax - a5 > nMaxDeviation ) nMeanMax = a5 + nMaxDeviation;
   return static_cast<Byte>(nMeanMax);
}

template<Border borderMode, decltype(simd_load_epi128) load, decltype(simd_store_epi128) store>
static FORCEINLINE void process_line(Byte *pDst, const Byte *pSrcp, const Byte *pSrc, const Byte *pSrcn, const __m128i &maxDeviation, int width) {
    auto zero = _mm_setzero_si128();

    __m128i mask_left, mask_right;
    if (borderMode == Border::Left) {
        mask_left = _mm_setr_epi8(0xFF, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00);
    } else if (borderMode == Border::Right) {
        mask_right = _mm_setr_epi8(00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 0xFF);
    }

    for ( int x = 0; x < width; x+=16 ) {
        __m128i up_left, up_center, up_right, middle_left, middle_right, down_left, down_center, down_right;

        if (borderMode == Border::Left) {
            up_left = load(reinterpret_cast<const __m128i*>(pSrcp + x));
            up_left = _mm_or_si128(_mm_slli_si128(up_left, 1), _mm_and_si128(up_left, mask_left));
        } else {
            up_left = simd_loadu_epi128(reinterpret_cast<const __m128i*>(pSrcp + x - 1));
        }

        up_center = load(reinterpret_cast<const __m128i*>(pSrcp + x));

        if (borderMode == Border::Right) {
            up_right = load(reinterpret_cast<const __m128i*>(pSrcp + x));
            up_right = _mm_or_si128(_mm_srli_si128(up_right, 1), _mm_and_si128(up_right, mask_right));
        } else {
            up_right = simd_loadu_epi128(reinterpret_cast<const __m128i*>(pSrcp + x + 1));
        }

        if (borderMode == Border::Left) {
            middle_left = load(reinterpret_cast<const __m128i*>(pSrc + x));
            middle_left = _mm_or_si128(_mm_slli_si128(middle_left, 1), _mm_and_si128(middle_left, mask_left));
        } else {
            middle_left = simd_loadu_epi128(reinterpret_cast<const __m128i*>(pSrc + x - 1));
        }

        if (borderMode == Border::Right) {
            middle_right = load(reinterpret_cast<const __m128i*>(pSrc + x));
            middle_right = _mm_or_si128(_mm_srli_si128(middle_right, 1), _mm_and_si128(middle_right, mask_right));
        } else {
            middle_right = simd_loadu_epi128(reinterpret_cast<const __m128i*>(pSrc + x + 1));
        }

        if (borderMode == Border::Left) {
            down_left = load(reinterpret_cast<const __m128i*>(pSrcn + x));
            down_left = _mm_or_si128(_mm_slli_si128(down_left, 1), _mm_and_si128(down_left, mask_left));
        } else {
            down_left = simd_loadu_epi128(reinterpret_cast<const __m128i*>(pSrcn + x - 1));
        }

        down_center = load(reinterpret_cast<const __m128i*>(pSrcn + x));


        if (borderMode == Border::Right) {
            down_right = load(reinterpret_cast<const __m128i*>(pSrcn + x));
            down_right = _mm_or_si128(_mm_srli_si128(down_right, 1), _mm_and_si128(down_right, mask_right));
        } else {
            down_right = simd_loadu_epi128(reinterpret_cast<const __m128i*>(pSrcn + x + 1));
        }

        auto up_left_lo = _mm_unpacklo_epi8(up_left, zero);
        auto up_left_hi = _mm_unpackhi_epi8(up_left, zero);

        auto up_center_lo = _mm_unpacklo_epi8(up_center, zero);
        auto up_center_hi = _mm_unpackhi_epi8(up_center, zero);

        auto up_right_lo = _mm_unpacklo_epi8(up_right, zero);
        auto up_right_hi = _mm_unpackhi_epi8(up_right, zero);

        auto middle_left_lo = _mm_unpacklo_epi8(middle_left, zero);
        auto middle_left_hi = _mm_unpackhi_epi8(middle_left, zero);

        auto middle_right_lo = _mm_unpacklo_epi8(middle_right, zero);
        auto middle_right_hi = _mm_unpackhi_epi8(middle_right, zero);

        auto down_left_lo = _mm_unpacklo_epi8(down_left, zero);
        auto down_left_hi = _mm_unpackhi_epi8(down_left, zero);

        auto down_center_lo = _mm_unpacklo_epi8(down_center, zero);
        auto down_center_hi = _mm_unpackhi_epi8(down_center, zero);

        auto down_right_lo = _mm_unpacklo_epi8(down_right, zero);
        auto down_right_hi = _mm_unpackhi_epi8(down_right, zero);

        auto sum_lo = _mm_add_epi16(up_left_lo, up_center_lo);
        sum_lo = _mm_add_epi16(sum_lo, up_right_lo);
        sum_lo = _mm_add_epi16(sum_lo, middle_left_lo);
        sum_lo = _mm_add_epi16(sum_lo, middle_right_lo);
        sum_lo = _mm_add_epi16(sum_lo, down_left_lo);
        sum_lo = _mm_add_epi16(sum_lo, down_center_lo);
        sum_lo = _mm_add_epi16(sum_lo, down_right_lo);

        auto sum_hi = _mm_add_epi16(up_left_hi, up_center_hi);
        sum_hi = _mm_add_epi16(sum_hi, up_right_hi);
        sum_hi = _mm_add_epi16(sum_hi, middle_left_hi);
        sum_hi = _mm_add_epi16(sum_hi, middle_right_hi);
        sum_hi = _mm_add_epi16(sum_hi, down_left_hi);
        sum_hi = _mm_add_epi16(sum_hi, down_center_hi);
        sum_hi = _mm_add_epi16(sum_hi, down_right_hi);

        sum_lo = _mm_srai_epi16(sum_lo, 3);
        sum_hi = _mm_srai_epi16(sum_hi, 3);

        auto result = _mm_packus_epi16(sum_lo, sum_hi);

        auto middle_center = simd_loadu_epi128(reinterpret_cast<const __m128i*>(pSrc + x));
        auto limit = _mm_adds_epu8(middle_center, maxDeviation);
        result = _mm_min_epu8(limit, _mm_max_epu8(middle_center, result));

        store(reinterpret_cast<__m128i*>(pDst+x), result);
    }
}

template<decltype(simd_load_epi128) load, decltype(simd_store_epi128) store>
static void generic_sse2(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nMaxDeviation, const int *pCoordinates, int nCoordinates, int nWidth, int nHeight) {
    const Byte *pSrcp = pSrc - nSrcPitch;
    const Byte *pSrcn = pSrc + nSrcPitch;

    UNUSED(nCoordinates); UNUSED(pCoordinates);
    auto max_dev_v = simd_set8_epi32(nMaxDeviation);
    int sse2_width = (nWidth - 1 - 16) / 16 * 16 + 16;
    /* top-left */
    process_line<Border::Left, load, store>(pDst, pSrc, pSrc, pSrcn, max_dev_v, 16);
    /* top */
    process_line<Border::None, load, store>(pDst + 16, pSrc+16, pSrc+16, pSrcn+16, max_dev_v, sse2_width - 16);

    /* top-right */
    process_line<Border::Right, load, store>(pDst + nWidth - 16, pSrc + nWidth - 16, pSrc + nWidth - 16, pSrcn + nWidth - 16, max_dev_v, 16);

    pDst  += nDstPitch;
    pSrcp += nSrcPitch;
    pSrc  += nSrcPitch;
    pSrcn += nSrcPitch;



    for ( int y = 1; y < nHeight-1; y++ )
    {
        /* left */
        process_line<Border::Left, load, store>(pDst, pSrcp, pSrc, pSrcn, max_dev_v, 16);
        /* center */
        process_line<Border::None, load, store>(pDst + 16, pSrcp+16, pSrc+16, pSrcn+16, max_dev_v, sse2_width - 16);

        /* right */
        process_line<Border::Right, load, store>(pDst + nWidth - 16, pSrcp + nWidth - 16, pSrc + nWidth - 16, pSrcn + nWidth - 16, max_dev_v, 16);

        pDst  += nDstPitch;
        pSrcp += nSrcPitch;
        pSrc  += nSrcPitch;
        pSrcn += nSrcPitch;
    }

    /* bottom-left */
    process_line<Border::Left, load, store>(pDst, pSrcp, pSrc, pSrc, max_dev_v, 16);
    /* bottom */
    process_line<Border::None, load, store>(pDst + 16, pSrcp+16, pSrc+16, pSrc+16, max_dev_v, sse2_width - 16);
    /* bottom-right */
    process_line<Border::Right, load, store>(pDst + nWidth - 16, pSrcp + nWidth - 16, pSrc + nWidth - 16, pSrc + nWidth - 16, max_dev_v, 16);
}


namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic { namespace Inflate {

Processor *inflate_c = &generic_c<meanMaxThresholded>;
Processor *inflate_sse2 = &generic_sse2<simd_loadu_epi128, simd_storeu_epi128>;
Processor *inflate_asse2 = &generic_sse2<simd_load_epi128, simd_store_epi128>;

} } } } }