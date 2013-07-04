#include "edgemask.h"
#include "../functions.h"
#include "../../../common/simd.h"

using namespace Filtering;

namespace Filtering { namespace MaskTools { namespace Filters { namespace Mask { namespace Edge {

inline Byte convolution(Byte a11, Byte a21, Byte a31, Byte a12, Byte a22, Byte a32, Byte a13, Byte a23, Byte a33, const Short matrix[10], int nLowThreshold, int nHighThreshold)
{
   return threshold<Byte, int>(abs((a11 * matrix[0] + a21 * matrix[1] + a31 * matrix[2] + 
                                    a12 * matrix[3] + a22 * matrix[4] + a32 * matrix[5] +
                                    a13 * matrix[6] + a23 * matrix[7] + a33 * matrix[8]) / matrix[9]), nLowThreshold, nHighThreshold);
}

inline Byte sobel(Byte a11, Byte a21, Byte a31, Byte a12, Byte a22, Byte a32, Byte a13, Byte a23, Byte a33, const Short matrix[10], int nLowThreshold, int nHighThreshold)
{
   UNUSED(a11); UNUSED(a13); UNUSED(a22); UNUSED(a31); UNUSED(a33); UNUSED(matrix); 
   return threshold<Byte, int>(abs( (int)a32 + a23 - a12 - a21 ) >> 1, nLowThreshold, nHighThreshold);
}

inline Byte roberts(Byte a11, Byte a21, Byte a31, Byte a12, Byte a22, Byte a32, Byte a13, Byte a23, Byte a33, const Short matrix[10], int nLowThreshold, int nHighThreshold)
{
   UNUSED(a11); UNUSED(a12); UNUSED(a13); UNUSED(a21); UNUSED(a31); UNUSED(a33); UNUSED(matrix); 
   return threshold<Byte, int>(abs( ((int)a22 << 1) - a32 - a23 ) >> 1, nLowThreshold, nHighThreshold);
}

inline Byte laplace(Byte a11, Byte a21, Byte a31, Byte a12, Byte a22, Byte a32, Byte a13, Byte a23, Byte a33, const Short matrix[10], int nLowThreshold, int nHighThreshold)
{
   UNUSED(matrix); 
   return threshold<Byte, int>(abs( ((int)a22 << 3) - a32 - a23 - a11 - a21 - a31 - a12 - a13 - a33 ) >> 3, nLowThreshold, nHighThreshold);
}

inline Byte morpho(Byte a11, Byte a21, Byte a31, Byte a12, Byte a22, Byte a32, Byte a13, Byte a23, Byte a33, const Short matrix[10], int nLowThreshold, int nHighThreshold)
{
   int nMin = a11, nMax = a11;

   UNUSED(matrix); 

   nMin = min<int>( nMin, a21 );
   nMax = max<int>( nMax, a21 );
   nMin = min<int>( nMin, a31 );
   nMax = max<int>( nMax, a31 );
   nMin = min<int>( nMin, a12 );
   nMax = max<int>( nMax, a12 );
   nMin = min<int>( nMin, a22 );
   nMax = max<int>( nMax, a22 );
   nMin = min<int>( nMin, a32 );
   nMax = max<int>( nMax, a32 );
   nMin = min<int>( nMin, a13 );
   nMax = max<int>( nMax, a13 );
   nMin = min<int>( nMin, a23 );
   nMax = max<int>( nMax, a23 );
   nMin = min<int>( nMin, a33 );
   nMax = max<int>( nMax, a33 );

   return threshold<Byte, int>( nMax - nMin, nLowThreshold, nHighThreshold );
}

inline Byte cartoon(Byte a11, Byte a21, Byte a31, Byte a12, Byte a22, Byte a32, Byte a13, Byte a23, Byte a33, const Short matrix[10], int nLowThreshold, int nHighThreshold)
{
   int val = ((int)a21 << 1) - a22 - a31;

   UNUSED(a11); UNUSED(a12); UNUSED(a13); UNUSED(a23); UNUSED(a32); UNUSED(a33); UNUSED(matrix); 

   return val > 0 ? 0 : threshold<Byte, int>( -val, nLowThreshold, nHighThreshold );
}

inline Byte prewitt(Byte a11, Byte a21, Byte a31, Byte a12, Byte a22, Byte a32, Byte a13, Byte a23, Byte a33, const Short matrix[10], int nLowThreshold, int nHighThreshold)
{
   const int p90 = a11 + a21 + a31 - a13 - a23 - a33;
   const int p180 = a11 + a12 + a13 - a31 - a32 - a33;
   const int p45 = a12 + a11 + a21 - a33 - a32 - a23;
   const int p135 = a13 + a12 + a23 - a31 - a32 - a21;

   const int max1 = max<int>( abs<int>( p90 ), abs<int>( p180 ) );
   const int max2 = max<int>( abs<int>( p45 ), abs<int>( p135 ) );
   const int maxv = max<int>( max1, max2 );

   UNUSED(a22); UNUSED(matrix); 

   return threshold<Byte, int>( maxv, nLowThreshold, nHighThreshold );
}

inline Byte half_prewitt(Byte a11, Byte a21, Byte a31, Byte a12, Byte a22, Byte a32, Byte a13, Byte a23, Byte a33, const Short matrix[10], int nLowThreshold, int nHighThreshold)
{
   const int p90 = a11 + 2 * a21 + a31 - a13 - 2 * a23 - a33;
   const int p180 = a11 + 2 * a12 + a13 - a31 - 2 * a32 - a33;
   const int maxv = max<int>( abs<int>( p90 ), abs<int>( p180 ) );

   UNUSED(a22); UNUSED(matrix);
   
   return threshold<Byte, int>( maxv, nLowThreshold, nHighThreshold );
}

class Thresholds {
   Byte nMinThreshold, nMaxThreshold;
public:
   Thresholds(Byte nMinThreshold, Byte nMaxThreshold) :
   nMinThreshold(nMinThreshold), nMaxThreshold(nMaxThreshold)
   {
   }

   int minpitch() const { return 0; }
   int maxpitch() const { return 0; }
   void nextRow() { }
   Byte min(int x) const { UNUSED(x); return nMinThreshold; }
   Byte max(int x) const { UNUSED(x); return nMaxThreshold; }
};

template<Filters::Mask::Operator op>
void mask_t(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, const Short matrix[10], int nLowThreshold, int nHighThreshold, int nWidth, int nHeight)
{
   Thresholds thresholds(static_cast<Byte>(nLowThreshold), static_cast<Byte>(nHighThreshold));

   Filters::Mask::generic_c<op, Thresholds>(pDst, nDstPitch, pSrc, nSrcPitch, thresholds, matrix, nWidth, nHeight);
}

template <CpuFlags flags>
static FORCEINLINE __m128i simd_packed_abs_epi16(__m128i a, __m128i b) {
    if (flags >= CPU_SSSE3) {
        auto absa = _mm_abs_epi16(a);
        auto absb = _mm_abs_epi16(b);
        return _mm_packus_epi16(absa, absb);
    } else {
        auto suba = _mm_sub_epi16(_mm_setzero_si128(), a);
        auto subb = _mm_sub_epi16(_mm_setzero_si128(), b);
        auto t1 = _mm_packus_epi16(a, b);
        auto t2 = _mm_packus_epi16(suba, subb);
        return _mm_max_epu8(t1, t2);
    }
}

static FORCEINLINE __m128i threshold_sse2(const __m128i &value, const __m128i &lowThresh, const __m128i &highThresh, const __m128i &v128) {
    auto sat = _mm_sub_epi8(value, v128);
    auto low = _mm_cmpgt_epi8(sat, lowThresh);
    auto high = _mm_cmpgt_epi8(sat, highThresh);
    auto result = _mm_and_si128(value, low);
    return _mm_or_si128(result, high);
}

template<CpuFlags flags, Border borderMode, decltype(simd_load_epi128) load, decltype(simd_store_epi128) store>
static FORCEINLINE void process_line_convolution_sse2(Byte *pDst, const Byte *pSrcp, const Byte *pSrc, const Byte *pSrcn, const Short matrix[10], const __m128i &lowThresh, const __m128i &highThresh, int width) {
    UNUSED(pSrcp);
    auto v128 = simd_set8_epi32(0x80);
    auto zero = _mm_setzero_si128();
    auto coef0 = _mm_set1_epi16(matrix[0]);
    auto coef1 = _mm_set1_epi16(matrix[1]);
    auto coef2 = _mm_set1_epi16(matrix[2]);
    auto coef3 = _mm_set1_epi16(matrix[3]);
    auto coef4 = _mm_set1_epi16(matrix[4]);
    auto coef5 = _mm_set1_epi16(matrix[5]);
    auto coef6 = _mm_set1_epi16(matrix[6]);
    auto coef7 = _mm_set1_epi16(matrix[7]);
    auto coef8 = _mm_set1_epi16(matrix[8]);
    
    auto divisor = _mm_set_epi32(0, 0, 0, simd_bit_scan_forward(matrix[9]));

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

        auto up_left_lo = _mm_unpacklo_epi8(up_left, zero);
        auto up_left_hi = _mm_unpackhi_epi8(up_left, zero);

        auto up_center_lo = _mm_unpacklo_epi8(up_center, zero);
        auto up_center_hi = _mm_unpackhi_epi8(up_center, zero);

        auto up_right_lo = _mm_unpacklo_epi8(up_right, zero);
        auto up_right_hi = _mm_unpackhi_epi8(up_right, zero);

        auto middle_left_lo = _mm_unpacklo_epi8(middle_left, zero);
        auto middle_left_hi = _mm_unpackhi_epi8(middle_left, zero);

        auto middle_center_lo = _mm_unpacklo_epi8(middle_center, zero);
        auto middle_center_hi = _mm_unpackhi_epi8(middle_center, zero);

        auto middle_right_lo = _mm_unpacklo_epi8(middle_right, zero);
        auto middle_right_hi = _mm_unpackhi_epi8(middle_right, zero);

        auto down_left_lo = _mm_unpacklo_epi8(down_left, zero);
        auto down_left_hi = _mm_unpackhi_epi8(down_left, zero);

        auto down_center_lo = _mm_unpacklo_epi8(down_center, zero);
        auto down_center_hi = _mm_unpackhi_epi8(down_center, zero);

        auto down_right_lo = _mm_unpacklo_epi8(down_right, zero);
        auto down_right_hi = _mm_unpackhi_epi8(down_right, zero);

        auto acc_lo = _mm_mullo_epi16(up_left_lo, coef0);
        acc_lo = _mm_add_epi16(acc_lo, _mm_mullo_epi16(up_center_lo, coef1));
        acc_lo = _mm_add_epi16(acc_lo, _mm_mullo_epi16(up_right_lo, coef2));
        acc_lo = _mm_add_epi16(acc_lo, _mm_mullo_epi16(middle_left_lo, coef3));
        acc_lo = _mm_add_epi16(acc_lo, _mm_mullo_epi16(middle_center_lo, coef4));
        acc_lo = _mm_add_epi16(acc_lo, _mm_mullo_epi16(middle_right_lo, coef5));
        acc_lo = _mm_add_epi16(acc_lo, _mm_mullo_epi16(down_left_lo, coef6));
        acc_lo = _mm_add_epi16(acc_lo, _mm_mullo_epi16(down_center_lo, coef7));
        acc_lo = _mm_add_epi16(acc_lo, _mm_mullo_epi16(down_right_lo, coef8));

        auto acc_hi = _mm_mullo_epi16(up_left_hi, coef0);
        acc_hi = _mm_add_epi16(acc_hi, _mm_mullo_epi16(up_center_hi, coef1));
        acc_hi = _mm_add_epi16(acc_hi, _mm_mullo_epi16(up_right_hi, coef2));
        acc_hi = _mm_add_epi16(acc_hi, _mm_mullo_epi16(middle_left_hi, coef3));
        acc_hi = _mm_add_epi16(acc_hi, _mm_mullo_epi16(middle_center_hi, coef4));
        acc_hi = _mm_add_epi16(acc_hi, _mm_mullo_epi16(middle_right_hi, coef5));
        acc_hi = _mm_add_epi16(acc_hi, _mm_mullo_epi16(down_left_hi, coef6));
        acc_hi = _mm_add_epi16(acc_hi, _mm_mullo_epi16(down_center_hi, coef7));
        acc_hi = _mm_add_epi16(acc_hi, _mm_mullo_epi16(down_right_hi, coef8));

        auto shift_lo = _mm_srai_epi16(acc_lo, 15);
        auto shift_hi = _mm_srai_epi16(acc_hi, 15);
        
        acc_lo = _mm_xor_si128(acc_lo, shift_lo);
        acc_hi = _mm_xor_si128(acc_hi, shift_hi);

        acc_lo = _mm_sub_epi16(acc_lo, shift_lo);
        acc_hi = _mm_sub_epi16(acc_hi, shift_hi);

        acc_lo = _mm_srl_epi16(acc_lo, divisor);
        acc_hi = _mm_srl_epi16(acc_hi, divisor);

        auto acc = _mm_packus_epi16(acc_lo, acc_hi);
        auto result = threshold_sse2(acc, lowThresh, highThresh, v128);

        store(reinterpret_cast<__m128i*>(pDst+x), result);
    }
}

template<CpuFlags flags, Border borderMode, decltype(simd_load_epi128) load, decltype(simd_store_epi128) store>
static FORCEINLINE void process_line_sobel_sse2(Byte *pDst, const Byte *pSrcp, const Byte *pSrc, const Byte *pSrcn, const Short matrix[10], const __m128i &lowThresh, const __m128i &highThresh, int width) {
    UNUSED(matrix);
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

        auto diff_lo = simd_abs_diff_epu16(pos_lo, neg_lo);
        auto diff_hi = simd_abs_diff_epu16(pos_hi, neg_hi);

        diff_lo = _mm_srai_epi16(diff_lo, 1);
        diff_hi = _mm_srai_epi16(diff_hi, 1);

        auto diff = _mm_packus_epi16(diff_lo, diff_hi);
        auto result = threshold_sse2(diff, lowThresh, highThresh, v128);

        store(reinterpret_cast<__m128i*>(pDst+x), result);
    }
}

template<CpuFlags flags, Border borderMode, decltype(simd_load_epi128) load, decltype(simd_store_epi128) store>
static FORCEINLINE void process_line_roberts_sse2(Byte *pDst, const Byte *pSrcp, const Byte *pSrc, const Byte *pSrcn, const Short matrix[10], const __m128i &lowThresh, const __m128i &highThresh, int width) {
    UNUSED(pSrcp);
    UNUSED(matrix);
    auto v128 = simd_set8_epi32(0x80);
    auto zero = _mm_setzero_si128();

    for (int x = 0; x < width; x+=16) {
        auto middle_center = load(reinterpret_cast<const __m128i*>(pSrc+x));
        auto middle_right = load_one_to_right<borderMode == Border::Right, load>(pSrc+x);

        auto down_center = load(reinterpret_cast<const __m128i*>(pSrcn+x));

        auto middle_center_lo = _mm_unpacklo_epi8(middle_center, zero);
        auto middle_center_hi = _mm_unpackhi_epi8(middle_center, zero);

        auto middle_right_lo = _mm_unpacklo_epi8(middle_right, zero);
        auto middle_right_hi = _mm_unpackhi_epi8(middle_right, zero);

        auto down_center_lo = _mm_unpacklo_epi8(down_center, zero);
        auto down_center_hi = _mm_unpackhi_epi8(down_center, zero);

        auto pos_lo = _mm_add_epi16(middle_center_lo, middle_center_lo);
        auto pos_hi = _mm_add_epi16(middle_center_hi, middle_center_hi);

        auto neg_lo = _mm_add_epi16(middle_right_lo, down_center_lo);
        auto neg_hi = _mm_add_epi16(middle_right_hi, down_center_hi);

        auto diff_lo = simd_abs_diff_epu16(pos_lo, neg_lo);
        auto diff_hi = simd_abs_diff_epu16(pos_hi, neg_hi);

        diff_lo = _mm_srai_epi16(diff_lo, 1);
        diff_hi = _mm_srai_epi16(diff_hi, 1);

        auto diff = _mm_packus_epi16(diff_lo, diff_hi);
        auto result = threshold_sse2(diff, lowThresh, highThresh, v128);

        store(reinterpret_cast<__m128i*>(pDst+x), result);
    }
}

template<CpuFlags flags, Border borderMode, decltype(simd_load_epi128) load, decltype(simd_store_epi128) store>
static FORCEINLINE void process_line_laplace_sse2(Byte *pDst, const Byte *pSrcp, const Byte *pSrc, const Byte *pSrcn, const Short matrix[10], const __m128i &lowThresh, const __m128i &highThresh, int width) {
    UNUSED(pSrcp);
    UNUSED(matrix);
    auto v128 = simd_set8_epi32(0x80);
    auto zero = _mm_setzero_si128();

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

        auto up_left_lo = _mm_unpacklo_epi8(up_left, zero);
        auto up_left_hi = _mm_unpackhi_epi8(up_left, zero);

        auto up_center_lo = _mm_unpacklo_epi8(up_center, zero);
        auto up_center_hi = _mm_unpackhi_epi8(up_center, zero);

        auto up_right_lo = _mm_unpacklo_epi8(up_right, zero);
        auto up_right_hi = _mm_unpackhi_epi8(up_right, zero);

        auto middle_left_lo = _mm_unpacklo_epi8(middle_left, zero);
        auto middle_left_hi = _mm_unpackhi_epi8(middle_left, zero);

        auto middle_center_lo = _mm_unpacklo_epi8(middle_center, zero);
        auto middle_center_hi = _mm_unpackhi_epi8(middle_center, zero);

        auto middle_right_lo = _mm_unpacklo_epi8(middle_right, zero);
        auto middle_right_hi = _mm_unpackhi_epi8(middle_right, zero);

        auto down_left_lo = _mm_unpacklo_epi8(down_left, zero);
        auto down_left_hi = _mm_unpackhi_epi8(down_left, zero);

        auto down_center_lo = _mm_unpacklo_epi8(down_center, zero);
        auto down_center_hi = _mm_unpackhi_epi8(down_center, zero);

        auto down_right_lo = _mm_unpacklo_epi8(down_right, zero);
        auto down_right_hi = _mm_unpackhi_epi8(down_right, zero);

        auto acc_lo = _mm_add_epi16(up_left_lo, up_center_lo);
        acc_lo = _mm_add_epi16(acc_lo, up_right_lo);
        acc_lo = _mm_add_epi16(acc_lo, middle_left_lo);
        acc_lo = _mm_add_epi16(acc_lo, middle_right_lo);
        acc_lo = _mm_add_epi16(acc_lo, down_left_lo);
        acc_lo = _mm_add_epi16(acc_lo, down_center_lo);
        acc_lo = _mm_add_epi16(acc_lo, down_right_lo);

        auto acc_hi = _mm_add_epi16(up_left_hi, up_center_hi);
        acc_hi = _mm_add_epi16(acc_hi, up_right_hi);
        acc_hi = _mm_add_epi16(acc_hi, middle_left_hi);
        acc_hi = _mm_add_epi16(acc_hi, middle_right_hi);
        acc_hi = _mm_add_epi16(acc_hi, down_left_hi);
        acc_hi = _mm_add_epi16(acc_hi, down_center_hi);
        acc_hi = _mm_add_epi16(acc_hi, down_right_hi);

        auto pos_lo = _mm_slli_epi16(middle_center_lo, 3);
        auto pos_hi = _mm_slli_epi16(middle_center_hi, 3);

        auto diff_lo = simd_abs_diff_epu16(pos_lo, acc_lo);
        auto diff_hi = simd_abs_diff_epu16(pos_hi, acc_hi);
        
        diff_lo = _mm_srai_epi16(diff_lo, 3);
        diff_hi = _mm_srai_epi16(diff_hi, 3);

        auto diff = _mm_packus_epi16(diff_lo, diff_hi);
        auto result = threshold_sse2(diff, lowThresh, highThresh, v128);

        store(reinterpret_cast<__m128i*>(pDst+x), result);
    }
}

template<CpuFlags flags, Border borderMode, decltype(simd_load_epi128) load, decltype(simd_store_epi128) store>
static FORCEINLINE void process_line_morpho_sse2(Byte *pDst, const Byte *pSrcp, const Byte *pSrc, const Byte *pSrcn, const Short matrix[10], const __m128i &lowThresh, const __m128i &highThresh, int width) {
    UNUSED(matrix);
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

template<CpuFlags flags, Border borderMode, decltype(simd_load_epi128) load, decltype(simd_store_epi128) store>
static FORCEINLINE void process_line_prewitt_sse2(Byte *pDst, const Byte *pSrcp, const Byte *pSrc, const Byte *pSrcn, const Short matrix[10], const __m128i &lowThresh, const __m128i &highThresh, int width) {
    UNUSED(matrix);
    auto v128 = simd_set8_epi32(0x80);
    auto zero = _mm_setzero_si128();

    for (int x = 0; x < width; x+=16) {
        auto up_left = load_one_to_left<borderMode == Border::Left, load>(pSrcp+x);
        auto up_center = load(reinterpret_cast<const __m128i*>(pSrcp+x));
        auto up_right = load_one_to_right<borderMode == Border::Right, load>(pSrcp+x);

        auto middle_left = load_one_to_left<borderMode == Border::Left, load>(pSrc+x);
        auto middle_right = load_one_to_right<borderMode == Border::Right, load>(pSrc+x);

        auto down_left = load_one_to_left<borderMode == Border::Left, load>(pSrcn+x);
        auto down_center = load(reinterpret_cast<const __m128i*>(pSrcn+x));
        auto down_right = load_one_to_right<borderMode == Border::Right, load>(pSrcn+x);

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

        auto a21_minus_a23_lo = _mm_sub_epi16(up_center_lo, down_center_lo); // a21 - a23
        auto a21_minus_a23_hi = _mm_sub_epi16(up_center_hi, down_center_hi);
        
        auto a11_minus_a33_lo = _mm_sub_epi16(up_left_lo, down_right_lo); // a11 - a33
        auto a11_minus_a33_hi = _mm_sub_epi16(up_left_hi, down_right_hi);

        auto t1_lo = _mm_add_epi16(a21_minus_a23_lo, a11_minus_a33_lo); // a11 + a21 - a23 - a33
        auto t1_hi = _mm_add_epi16(a21_minus_a23_hi, a11_minus_a33_hi);

        auto a12_minus_a32_lo = _mm_sub_epi16(middle_left_lo, middle_right_lo); // a12 - a32
        auto a12_minus_a32_hi = _mm_sub_epi16(middle_left_hi, middle_right_hi);

        auto a13_minus_a31_lo = _mm_sub_epi16(down_left_lo, up_right_lo); // a13 - a31
        auto a13_minus_a31_hi = _mm_sub_epi16(down_left_hi, up_right_hi);

        auto t2_lo = _mm_add_epi16(a12_minus_a32_lo, a13_minus_a31_lo); //a13 + a12 - a31 - a32
        auto t2_hi = _mm_add_epi16(a12_minus_a32_hi, a13_minus_a31_hi);

        auto p135_lo = _mm_sub_epi16(t2_lo, a21_minus_a23_lo); //a13 + a12 + a23 - a31 - a32 - a21
        auto p135_hi = _mm_sub_epi16(t2_hi, a21_minus_a23_hi);

        auto p180_lo = _mm_add_epi16(t2_lo, a11_minus_a33_lo); //a11+ a12+ a13 - a31 - a32 - a33
        auto p180_hi = _mm_add_epi16(t2_hi, a11_minus_a33_hi);

        auto p90_lo = _mm_sub_epi16(a13_minus_a31_lo, t1_lo); // a13 - a31 - a11 - a21 + a23 + a33 //negative
        auto p90_hi = _mm_sub_epi16(a13_minus_a31_hi, t1_hi);

        auto p45_lo = _mm_add_epi16(t1_lo, a12_minus_a32_lo); // a12 + a11 + a21 - a33 - a32 - a23
        auto p45_hi = _mm_add_epi16(t1_hi, a12_minus_a32_hi);

        auto p45 = simd_packed_abs_epi16<flags>(p45_lo, p45_hi);
        auto p90 = simd_packed_abs_epi16<flags>(p90_lo, p90_hi);
        auto p135 = simd_packed_abs_epi16<flags>(p135_lo, p135_hi);
        auto p180 = simd_packed_abs_epi16<flags>(p180_lo, p180_hi);

        auto max1 = _mm_max_epu8(p45, p90);
        auto max2 = _mm_max_epu8(p135, p180);

        auto result = _mm_max_epu8(max1, max2);

        result = threshold_sse2(result, lowThresh, highThresh, v128);

        store(reinterpret_cast<__m128i*>(pDst+x), result);
    }
}

template<CpuFlags flags, Border borderMode, decltype(simd_load_epi128) load, decltype(simd_store_epi128) store>
static FORCEINLINE void process_line_half_prewitt_sse2(Byte *pDst, const Byte *pSrcp, const Byte *pSrc, const Byte *pSrcn, const Short matrix[10], const __m128i &lowThresh, const __m128i &highThresh, int width) {
    UNUSED(matrix);
    auto v128 = simd_set8_epi32(0x80);
    auto zero = _mm_setzero_si128();

    for (int x = 0; x < width; x+=16) {
        auto up_left = load_one_to_left<borderMode == Border::Left, load>(pSrcp+x);
        auto up_center = load(reinterpret_cast<const __m128i*>(pSrcp+x));
        auto up_right = load_one_to_right<borderMode == Border::Right, load>(pSrcp+x);

        auto middle_left = load_one_to_left<borderMode == Border::Left, load>(pSrc+x);
        auto middle_right = load_one_to_right<borderMode == Border::Right, load>(pSrc+x);

        auto down_left = load_one_to_left<borderMode == Border::Left, load>(pSrcn+x);
        auto down_center = load(reinterpret_cast<const __m128i*>(pSrcn+x));
        auto down_right = load_one_to_right<borderMode == Border::Right, load>(pSrcn+x);

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

        //a11 + 2 * (a21 - a23) + a31 - a13 - a33
        auto t1_lo = _mm_sub_epi16(up_center_lo, down_center_lo); //2 * (a21 - a23)
        auto t1_hi = _mm_sub_epi16(up_center_hi, down_center_hi);
        t1_lo = _mm_slli_epi16(t1_lo, 1);
        t1_hi = _mm_slli_epi16(t1_hi, 1);
        
        auto t2_lo = _mm_sub_epi16(up_left_lo, down_left_lo); //a11 - a13
        auto t2_hi = _mm_sub_epi16(up_left_hi, down_left_hi);
        
        auto t3_lo = _mm_sub_epi16(up_right_lo, down_right_lo); //a31 - a33
        auto t3_hi = _mm_sub_epi16(up_right_hi, down_right_hi);

        t1_lo = _mm_add_epi16(t1_lo, t2_lo);
        t1_hi = _mm_add_epi16(t1_hi, t2_hi);

        auto p90_lo = _mm_add_epi16(t1_lo, t3_lo);
        auto p90_hi = _mm_add_epi16(t1_hi, t3_hi);

        //a11 + 2 * (a12 - a32) + a13 - a31 - a33
        t1_lo = _mm_sub_epi16(middle_left_lo, middle_right_lo); //2 * (a12 - a32)
        t1_hi = _mm_sub_epi16(middle_left_hi, middle_right_hi);
        t1_lo = _mm_slli_epi16(t1_lo, 1);
        t1_hi = _mm_slli_epi16(t1_hi, 1);

        t2_lo = _mm_sub_epi16(up_left_lo, up_right_lo); //a11 - a31
        t2_hi = _mm_sub_epi16(up_left_hi, up_right_hi);

        t3_lo = _mm_sub_epi16(down_left_lo, down_right_lo); //a13 - a33
        t3_hi = _mm_sub_epi16(down_left_hi, down_right_hi);

        t1_lo = _mm_add_epi16(t1_lo, t2_lo);
        t1_hi = _mm_add_epi16(t1_hi, t2_hi);

        auto p180_lo = _mm_add_epi16(t1_lo, t3_lo);
        auto p180_hi = _mm_add_epi16(t1_hi, t3_hi);

        auto p90 = simd_packed_abs_epi16<flags>(p90_lo, p90_hi);
        auto p180 = simd_packed_abs_epi16<flags>(p180_lo, p180_hi);

        auto result = _mm_max_epu8(p90, p180);

        result = threshold_sse2(result, lowThresh, highThresh, v128);

        store(reinterpret_cast<__m128i*>(pDst+x), result);
    }
}

using namespace Filters::Mask;

extern "C" Processor Edge_sobel8_mmx;
extern "C" Processor Edge_sobel8_sse2;
extern "C" Processor Edge_roberts8_mmx;
extern "C" Processor Edge_roberts8_sse2;
extern "C" Processor Edge_laplace8_mmx;
extern "C" Processor Edge_laplace8_sse2;
extern "C" Processor Edge_morpho8_isse;
extern "C" Processor Edge_morpho8_sse2;
extern "C" Processor Edge_convolution8_mmx;
extern "C" Processor Edge_convolution8_sse2;
extern "C" Processor Edge_prewitt8_isse;
extern "C" Processor Edge_prewitt8_sse2;
extern "C" Processor Edge_prewitt8_ssse3;
extern "C" Processor Edge_half_prewitt8_isse;
extern "C" Processor Edge_half_prewitt8_sse2;
extern "C" Processor Edge_half_prewitt8_ssse3;

Processor *convolution_c = &mask_t<convolution>;
Processor *convolution_sse2 = &generic_sse2<
    process_line_convolution_sse2<CPU_SSE2, Border::Left, simd_loadu_epi128, simd_storeu_epi128>,
    process_line_convolution_sse2<CPU_SSE2, Border::None, simd_loadu_epi128, simd_storeu_epi128>,
    process_line_convolution_sse2<CPU_SSE2, Border::Right, simd_loadu_epi128, simd_storeu_epi128>
>;

Processor *sobel_c = &mask_t<sobel>;
Processor *sobel_sse2 = &generic_sse2<
    process_line_sobel_sse2<CPU_SSE2, Border::Left, simd_loadu_epi128, simd_storeu_epi128>,
    process_line_sobel_sse2<CPU_SSE2, Border::None, simd_loadu_epi128, simd_storeu_epi128>,
    process_line_sobel_sse2<CPU_SSE2, Border::Right, simd_loadu_epi128, simd_storeu_epi128>
>;

Processor *roberts_c = &mask_t<roberts>;
Processor *roberts_sse2 = &generic_sse2<
    process_line_roberts_sse2<CPU_SSE2, Border::Left, simd_loadu_epi128, simd_storeu_epi128>,
    process_line_roberts_sse2<CPU_SSE2, Border::None, simd_loadu_epi128, simd_storeu_epi128>,
    process_line_roberts_sse2<CPU_SSE2, Border::Right, simd_loadu_epi128, simd_storeu_epi128>
>;

Processor *laplace_c = &mask_t<laplace>;
Processor *laplace_sse2 = &generic_sse2<
    process_line_laplace_sse2<CPU_SSE2, Border::Left, simd_loadu_epi128, simd_storeu_epi128>,
    process_line_laplace_sse2<CPU_SSE2, Border::None, simd_loadu_epi128, simd_storeu_epi128>,
    process_line_laplace_sse2<CPU_SSE2, Border::Right, simd_loadu_epi128, simd_storeu_epi128>
>;

Processor *prewitt_c = &mask_t<prewitt>;
Processor *prewitt_sse2 = &generic_sse2<
    process_line_prewitt_sse2<CPU_SSE2, Border::Left, simd_loadu_epi128, simd_storeu_epi128>,
    process_line_prewitt_sse2<CPU_SSE2, Border::None, simd_loadu_epi128, simd_storeu_epi128>,
    process_line_prewitt_sse2<CPU_SSE2, Border::Right, simd_loadu_epi128, simd_storeu_epi128>
>;
Processor *prewitt_ssse3 = &generic_sse2<
    process_line_prewitt_sse2<CPU_SSSE3, Border::Left, simd_loadu_epi128, simd_storeu_epi128>,
    process_line_prewitt_sse2<CPU_SSSE3, Border::None, simd_loadu_epi128, simd_storeu_epi128>,
    process_line_prewitt_sse2<CPU_SSSE3, Border::Right, simd_loadu_epi128, simd_storeu_epi128>
>;

Processor *half_prewitt_c = &mask_t<half_prewitt>;
Processor *half_prewitt_sse2 = &generic_sse2<
    process_line_half_prewitt_sse2<CPU_SSE2, Border::Left, simd_loadu_epi128, simd_storeu_epi128>,
    process_line_half_prewitt_sse2<CPU_SSE2, Border::None, simd_loadu_epi128, simd_storeu_epi128>,
    process_line_half_prewitt_sse2<CPU_SSE2, Border::Right, simd_loadu_epi128, simd_storeu_epi128>
>;
Processor *half_prewitt_ssse3 = &generic_sse2<
    process_line_half_prewitt_sse2<CPU_SSSE3, Border::Left, simd_loadu_epi128, simd_storeu_epi128>,
    process_line_half_prewitt_sse2<CPU_SSSE3, Border::None, simd_loadu_epi128, simd_storeu_epi128>,
    process_line_half_prewitt_sse2<CPU_SSSE3, Border::Right, simd_loadu_epi128, simd_storeu_epi128>
>;

Processor *cartoon_c = &mask_t<cartoon>;

Processor *morpho_c = &mask_t<morpho>;
Processor *morpho_sse2 = &generic_sse2<
    process_line_morpho_sse2<CPU_SSE2, Border::Left, simd_loadu_epi128, simd_storeu_epi128>,
    process_line_morpho_sse2<CPU_SSE2, Border::None, simd_loadu_epi128, simd_storeu_epi128>,
    process_line_morpho_sse2<CPU_SSE2, Border::Right, simd_loadu_epi128, simd_storeu_epi128>
    >;

} } } } }