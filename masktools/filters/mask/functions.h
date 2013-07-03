#ifndef __Mt_MaskFunctions_H__
#define __Mt_MaskFunctions_H__

#include "../../../common/utils/utils.h"
#include "../../common/simd.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Mask {

typedef Byte (Operator)(Byte, Byte, Byte, Byte, Byte, Byte, Byte, Byte, Byte, const Short matrix[10], int nLowThreshold, int nHighThreshold);
typedef void (ProcessLineSse2)(Byte *pDst, const Byte *pSrcp, const Byte *pSrc, const Byte *pSrcn, const __m128i &lowThresh, const __m128i &highThresh, int width);

template<Operator op, class T>
void generic_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, T &thresholds, const Short matrix[10], int nWidth, int nHeight)
{
   const Byte *pSrcp = pSrc - nSrcPitch;
   const Byte *pSrcn = pSrc + nSrcPitch;

   /* top-left */
   pDst[0] = op(pSrc[0], pSrc[0], pSrc[1], pSrc[0], pSrc[0], pSrc[1], pSrcn[0], pSrcn[0], pSrcn[1], matrix, thresholds.min(0), thresholds.max(0));

   /* top */
   for ( int x = 1; x < nWidth - 1; x++ )
      pDst[x] = op(pSrc[x-1], pSrc[x], pSrc[x+1], pSrc[x-1], pSrc[x], pSrc[x+1], pSrcn[x-1], pSrcn[x], pSrcn[x+1], matrix, thresholds.min(x), thresholds.max(x));

   /* top-right */
   pDst[nWidth-1] = op(pSrc[nWidth-2], pSrc[nWidth-1], pSrc[nWidth-1], pSrc[nWidth-2], pSrc[nWidth-1], pSrc[nWidth-1], pSrcn[nWidth-2], pSrcn[nWidth-1], pSrcn[nWidth-1], matrix, thresholds.min(nWidth-1), thresholds.max(nWidth-1));

   pDst  += nDstPitch;
   pSrcp += nSrcPitch;
   pSrc  += nSrcPitch;
   pSrcn += nSrcPitch;

   thresholds.nextRow();

   for ( int y = 1; y < nHeight - 1; y++ )
   {
      /* left */
      pDst[0] = op(pSrcp[0], pSrcp[0], pSrcp[1], pSrc[0], pSrc[0], pSrc[1], pSrcn[0], pSrcn[0], pSrcn[1], matrix, thresholds.min(0), thresholds.max(0));

      /* center */
      for ( int x = 1; x < nWidth - 1; x++ )
         pDst[x] = op(pSrcp[x-1], pSrcp[x], pSrcp[x+1], pSrc[x-1], pSrc[x], pSrc[x+1], pSrcn[x-1], pSrcn[x], pSrcn[x+1], matrix, thresholds.min(x), thresholds.max(x));

      /* right */
      pDst[nWidth-1] = op(pSrcp[nWidth-2], pSrcp[nWidth-1], pSrcp[nWidth-1], pSrc[nWidth-2], pSrc[nWidth-1], pSrc[nWidth-1], pSrcn[nWidth-2], pSrcn[nWidth-1], pSrcn[nWidth-1], matrix, thresholds.min(nWidth-1), thresholds.max(nWidth-1));

      pDst  += nDstPitch;
      pSrcp += nSrcPitch;
      pSrc  += nSrcPitch;
      pSrcn += nSrcPitch;

      thresholds.nextRow();
   }

   /* bottom-left */
   pDst[0] = op(pSrcp[0], pSrcp[0], pSrcp[1], pSrc[0], pSrc[0], pSrc[1], pSrc[0], pSrc[0], pSrc[1], matrix, thresholds.min(0), thresholds.max(0));

   /* bottom */
   for ( int x = 1; x < nWidth - 1; x++ )
      pDst[x] = op(pSrcp[x-1], pSrcp[x], pSrcp[x+1], pSrc[x-1], pSrc[x], pSrc[x+1], pSrc[x-1], pSrc[x], pSrc[x+1], matrix, thresholds.min(x), thresholds.max(x));

   /* bottom-right */
   pDst[nWidth-1] = op(pSrcp[nWidth-2], pSrcp[nWidth-1], pSrcp[nWidth-1], pSrc[nWidth-2], pSrc[nWidth-1], pSrc[nWidth-1], pSrc[nWidth-2], pSrc[nWidth-1], pSrc[nWidth-1], matrix, thresholds.min(nWidth-1), thresholds.max(nWidth-1));
}

static FORCEINLINE __m128i threshold_sse2(const __m128i &value, const __m128i &lowThresh, const __m128i &highThresh, const __m128i &v128) {
    auto sat = _mm_sub_epi8(value, v128);
    auto low = _mm_cmpgt_epi8(sat, lowThresh);
    auto high = _mm_cmpgt_epi8(sat, highThresh);
    auto result = _mm_and_si128(value, low);
    return _mm_or_si128(result, high);
}


template<Border borderMode, decltype(simd_load_epi128) load, decltype(simd_store_epi128) store>
static FORCEINLINE void process_line_sobel_sse2(Byte *pDst, const Byte *pSrcp, const Byte *pSrc, const Byte *pSrcn, const __m128i &lowThresh, const __m128i &highThresh, int width) {
    auto v128 = simd_set8_epi32(0x80);
    auto zero = _mm_setzero_si128();

    for (int x = 0; x < width; x+=16) {
        auto up_center = load(reinterpret_cast<const __m128i*>(pSrcp+x));

        auto middle_left = load_one_to_left<borderMode == Border::Left, load>(pSrc+x);
        auto middle_right = load_one_to_right<borderMode == Border::Right, load>(pSrc+x);

        auto down_center = load(reinterpret_cast<const __m128i*>(pSrcn+x));

        auto up_center_lo = _mm_unpacklo_epi8(up_center, zero);
        auto up_center_hi = _mm_unpackhi_epi8(up_center, zero);

        auto middle_left_lo = _mm_unpacklo_epi8(middle_left, zero);
        auto middle_left_hi = _mm_unpackhi_epi8(middle_left, zero);

        auto middle_right_lo = _mm_unpacklo_epi8(middle_right, zero);
        auto middle_right_hi = _mm_unpackhi_epi8(middle_right, zero);

        auto down_center_lo = _mm_unpacklo_epi8(down_center, zero);
        auto down_center_hi = _mm_unpackhi_epi8(down_center, zero);

        auto pos_lo = _mm_add_epi16(middle_right_lo, down_center_lo);
        auto pos_hi = _mm_add_epi16(middle_right_hi, down_center_hi);
        
        auto neg_lo = _mm_add_epi16(middle_left_lo, up_center_lo);
        auto neg_hi = _mm_add_epi16(middle_left_hi, up_center_hi);
        
        //todo: use ssse3 _mm_abs_epi16?
        auto gt_lo = _mm_subs_epu16(pos_lo, neg_lo);
        auto gt_hi = _mm_subs_epu16(pos_hi, neg_hi);
        
        auto lt_lo = _mm_subs_epu16(neg_lo, pos_lo);
        auto lt_hi = _mm_subs_epu16(neg_hi, pos_hi);
        
        auto diff_lo = _mm_add_epi16(gt_lo, lt_lo);
        auto diff_hi = _mm_add_epi16(gt_hi, lt_hi);
        
        diff_lo = _mm_srai_epi16(diff_lo, 1);
        diff_hi = _mm_srai_epi16(diff_hi, 1);
        
        auto diff = _mm_packus_epi16(diff_lo, diff_hi);
        auto result = threshold_sse2(diff, lowThresh, highThresh, v128);

        store(reinterpret_cast<__m128i*>(pDst+x), result);
    }
}

template<Border borderMode,decltype(simd_load_epi128) load, decltype(simd_store_epi128) store>
static FORCEINLINE void process_line_morpho_sse2(Byte *pDst, const Byte *pSrcp, const Byte *pSrc, const Byte *pSrcn, const __m128i &lowThresh, const __m128i &highThresh, int width) {
    auto v128 = simd_set8_epi32(0x80);

    for (int x = 0; x < width; x+=16) {
        auto up_left = load_one_to_left<borderMode == Border::Left, load>(pSrcp+x);
        auto up_center = load(reinterpret_cast<const __m128i*>(pSrcp+x));
        auto up_right = load_one_to_right<borderMode == Border::Right, load>(pSrcp+x);

        auto middle_left = load_one_to_left<borderMode == Border::Left, load>(pSrc+x);
        auto middle_center = load(reinterpret_cast<const __m128i*>(pSrc+x));
        auto middle_right = load_one_to_right<borderMode == Border::Right, load>(pSrc+x);

        auto down_left = load_one_to_left<borderMode == Border::Left, load>(pSrcn+x);
        auto down_center = load(reinterpret_cast<const __m128i*>(pSrcn+x));
        auto down_right = load_one_to_right<borderMode == Border::Right, load>(pSrcn+x);

        auto maxv = _mm_max_epu8(up_left, up_center);
        maxv = _mm_max_epu8(maxv, up_right);
        maxv = _mm_max_epu8(maxv, middle_left);
        maxv = _mm_max_epu8(maxv, middle_center);
        maxv = _mm_max_epu8(maxv, middle_right);
        maxv = _mm_max_epu8(maxv, down_left);
        maxv = _mm_max_epu8(maxv, down_center);
        maxv = _mm_max_epu8(maxv, down_right);

        auto minv = _mm_min_epu8(up_left, up_center);
        minv = _mm_min_epu8(minv, up_right);
        minv = _mm_min_epu8(minv, middle_left);
        minv = _mm_min_epu8(minv, middle_center);
        minv = _mm_min_epu8(minv, middle_right);
        minv = _mm_min_epu8(minv, down_left);
        minv = _mm_min_epu8(minv, down_center);
        minv = _mm_min_epu8(minv, down_right);

        auto diff = _mm_sub_epi8(maxv, minv);
        auto result = threshold_sse2(diff, lowThresh, highThresh, v128);

        store(reinterpret_cast<__m128i*>(pDst+x), result);
    }
}

template<ProcessLineSse2 process_line_left, ProcessLineSse2 process_line, ProcessLineSse2 process_line_right>
static void generic_sse2(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, const Short matrix[10], int nLowThreshold, int nHighThreshold, int nWidth, int nHeight) {
    const Byte *pSrcp = pSrc - nSrcPitch;
    const Byte *pSrcn = pSrc + nSrcPitch;

    UNUSED(matrix);
    auto v128 = simd_set8_epi32(128);
    auto low_thr_v = simd_set8_epi32(nLowThreshold);
    low_thr_v = _mm_sub_epi8(low_thr_v, v128);
    auto high_thr_v = simd_set8_epi32(nHighThreshold);
    high_thr_v = _mm_sub_epi8(high_thr_v, v128);

    int sse2_width = (nWidth - 1 - 16) / 16 * 16 + 16;
    /* top-left */
    process_line_left(pDst, pSrc, pSrc, pSrcn, low_thr_v, high_thr_v, 16);
    /* top */
    process_line(pDst + 16, pSrc+16, pSrc+16, pSrcn+16, low_thr_v, high_thr_v, sse2_width - 16);

    /* top-right */
    process_line_right(pDst + nWidth - 16, pSrc + nWidth - 16, pSrc + nWidth - 16, pSrcn + nWidth - 16, low_thr_v, high_thr_v, 16);

    pDst  += nDstPitch;
    pSrcp += nSrcPitch;
    pSrc  += nSrcPitch;
    pSrcn += nSrcPitch;

    for ( int y = 1; y < nHeight-1; y++ )
    {
        /* left */
        process_line_left(pDst, pSrcp, pSrc, pSrcn, low_thr_v, high_thr_v, 16);
        /* center */
        process_line(pDst + 16, pSrcp+16, pSrc+16, pSrcn+16, low_thr_v, high_thr_v, sse2_width - 16);
        /* right */
        process_line_right(pDst + nWidth - 16, pSrcp + nWidth - 16, pSrc + nWidth - 16, pSrcn + nWidth - 16, low_thr_v, high_thr_v, 16);

        pDst  += nDstPitch;
        pSrcp += nSrcPitch;
        pSrc  += nSrcPitch;
        pSrcn += nSrcPitch;
    }

    /* bottom-left */
    process_line_left(pDst, pSrcp, pSrc, pSrc, low_thr_v, high_thr_v, 16);
    /* bottom */
    process_line(pDst + 16, pSrcp+16, pSrc+16, pSrc+16, low_thr_v, high_thr_v, sse2_width - 16);
    /* bottom-right */
    process_line_right(pDst + nWidth - 16, pSrcp + nWidth - 16, pSrc + nWidth - 16, pSrc + nWidth - 16, low_thr_v, high_thr_v, 16);
}

} } } }

#endif