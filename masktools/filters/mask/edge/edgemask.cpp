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

        auto diff_lo = simd_abs_diff_epu16(pos_lo, neg_lo);
        auto diff_hi = simd_abs_diff_epu16(pos_hi, neg_hi);

        diff_lo = _mm_srai_epi16(diff_lo, 1);
        diff_hi = _mm_srai_epi16(diff_hi, 1);

        auto diff = _mm_packus_epi16(diff_lo, diff_hi);
        auto result = threshold_sse2(diff, lowThresh, highThresh, v128);

        store(reinterpret_cast<__m128i*>(pDst+x), result);
    }
}

template<Border borderMode, decltype(simd_load_epi128) load, decltype(simd_store_epi128) store>
static FORCEINLINE void process_line_roberts_sse2(Byte *pDst, const Byte *pSrcp, const Byte *pSrc, const Byte *pSrcn, const __m128i &lowThresh, const __m128i &highThresh, int width) {
    UNUSED(pSrcp);
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

template<Border borderMode, decltype(simd_load_epi128) load, decltype(simd_store_epi128) store>
static FORCEINLINE void process_line_laplace_sse2(Byte *pDst, const Byte *pSrcp, const Byte *pSrc, const Byte *pSrcn, const __m128i &lowThresh, const __m128i &highThresh, int width) {
    UNUSED(pSrcp);
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
Processor *convolution8_mmx = &Edge_convolution8_mmx;
Processor *convolution8_sse2 = &Edge_convolution8_sse2;

Processor *sobel_c = &mask_t<sobel>;
Processor *sobel_sse2 = &generic_sse2<
    process_line_sobel_sse2<Border::Left, simd_loadu_epi128, simd_storeu_epi128>,
    process_line_sobel_sse2<Border::None, simd_loadu_epi128, simd_storeu_epi128>,
    process_line_sobel_sse2<Border::Right, simd_loadu_epi128, simd_storeu_epi128>
>;

Processor *roberts_c = &mask_t<roberts>;
Processor *roberts_sse2 = &generic_sse2<
    process_line_roberts_sse2<Border::Left, simd_loadu_epi128, simd_storeu_epi128>,
    process_line_roberts_sse2<Border::None, simd_loadu_epi128, simd_storeu_epi128>,
    process_line_roberts_sse2<Border::Right, simd_loadu_epi128, simd_storeu_epi128>
>;

Processor *laplace_c = &mask_t<laplace>;
Processor *laplace_sse2 = &generic_sse2<
    process_line_laplace_sse2<Border::Left, simd_loadu_epi128, simd_storeu_epi128>,
    process_line_laplace_sse2<Border::None, simd_loadu_epi128, simd_storeu_epi128>,
    process_line_laplace_sse2<Border::Right, simd_loadu_epi128, simd_storeu_epi128>
>;

Processor *prewitt_c = &mask_t<prewitt>;
Processor *prewitt8_isse = &Edge_prewitt8_isse;
Processor *prewitt8_sse2 = &Edge_prewitt8_sse2;
Processor *prewitt8_ssse3 = &Edge_prewitt8_ssse3;

Processor *half_prewitt_c = &mask_t<half_prewitt>;
Processor *half_prewitt8_isse = &Edge_half_prewitt8_isse;
Processor *half_prewitt8_sse2 = &Edge_half_prewitt8_sse2;
Processor *half_prewitt8_ssse3 = &Edge_half_prewitt8_ssse3;

Processor *cartoon_c = &mask_t<cartoon>;

Processor *morpho_c = &mask_t<morpho>;
Processor *morpho_sse2 = &generic_sse2<
    process_line_morpho_sse2<Border::Left, simd_loadu_epi128, simd_storeu_epi128>,
    process_line_morpho_sse2<Border::None, simd_loadu_epi128, simd_storeu_epi128>,
    process_line_morpho_sse2<Border::Right, simd_loadu_epi128, simd_storeu_epi128>
    >;

} } } } }