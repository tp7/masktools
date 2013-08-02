#include "merge16.h"
#include "../../common/simd.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Merge16 {

enum MaskMode {
    MASK420,
    MASK444
};

__forceinline static Word get_value_stacked_c(const Byte *pMsb, const Byte *pLsb, int x) {
    return (Word(pMsb[x]) << 8) + pLsb[x];
}

__forceinline static Word get_mask_stacked_c(const Byte *pMsb, const Byte *pLsb, int, int x) {
    return get_value_stacked_c(pMsb, pLsb, x);
}

__forceinline static Word get_mask_420_stacked_c(const Byte *pMsb, const Byte *pLsb, int pitch, int x) {
    x = x*2;
    return ((int((get_value_stacked_c(pMsb, pLsb, x)) + get_value_stacked_c(pMsb + pitch, pLsb + pitch, x) + 1) >> 1) +
        ((int(get_value_stacked_c(pMsb, pLsb, x+1)) + get_value_stacked_c(pMsb + pitch, pLsb + pitch, x+1) + 1) >> 1) + 1) >> 1;
}


template<MaskMode mode>
void merge16_t_stacked_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
                 const Byte *pMask, ptrdiff_t nMaskPitch, int nWidth, int nHeight)
{
    auto pDstLsb = pDst + nDstPitch * nHeight / 2;
    auto pSrc1Lsb = pSrc1 + nSrc1Pitch * nHeight / 2;
    auto pMaskLsb = pMask + nMaskPitch * nHeight / (mode == MASK420 ? 1 : 2);

    for ( int y = 0; y < nHeight / 2; ++y )
    {
        for ( int x = 0; x < nWidth; ++x ) {
            Word dst = get_value_stacked_c(pDst, pDstLsb, x);
            Word src = get_value_stacked_c(pSrc1, pSrc1Lsb, x);
            Word mask;

            if (mode == MASK420) {
                mask = get_mask_420_stacked_c(pMask, pMaskLsb, nMaskPitch, x);
            } else {
                mask = get_mask_stacked_c(pMask, pMaskLsb, nMaskPitch, x);
            }

            Word output = 0;

            if (mask == 0) {
                output = dst;
            } else if (mask == 65535) {
                output = src;
            } else {
                output = dst +(((src - dst) * (mask >> 1)) >> 15);
            }

            pDst[x] = output >> 8;
            pDstLsb[x] = output & 0xFF;
        }
        pDst += nDstPitch;
        pDstLsb += nDstPitch;
        pSrc1 += nSrc1Pitch;
        pSrc1Lsb += nSrc1Pitch;

        if (mode == MASK420) {
            pMask += nMaskPitch * 2;
            pMaskLsb += nMaskPitch * 2;
        } else {
            pMask += nMaskPitch;
            pMaskLsb += nMaskPitch;
        }
    }
}

MT_FORCEINLINE static __m128i get_value_stacked_sse2(const Byte *pMsb, const Byte *pLsb, int x) {
    auto msb = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(pMsb+x));
    auto lsb = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(pLsb+x));
    return _mm_unpacklo_epi8(lsb, msb);
}

MT_FORCEINLINE static __m128i get_mask_stacked_sse2(const Byte *pMsb, const Byte *pLsb, int, int x) {
    return get_value_stacked_sse2(pMsb, pLsb, x);
}

MT_FORCEINLINE static __m128i get_mask_420_stacked_sse2(const Byte *pMsb, const Byte *pLsb, int pitch, int x) {
    x = x*2;

    auto row1_lo = get_value_stacked_sse2(pMsb, pLsb, x);
    auto row1_hi = get_value_stacked_sse2(pMsb, pLsb, x+8);
    auto row2_lo = get_value_stacked_sse2(pMsb+pitch, pLsb+pitch, x);
    auto row2_hi = get_value_stacked_sse2(pMsb+pitch, pLsb+pitch, x+8);

    auto avg_lo = _mm_avg_epu16(row1_lo, row2_lo);
    auto avg_hi = _mm_avg_epu16(row1_hi, row2_hi);

    auto avg_lo_sh = _mm_srli_si128(avg_lo, 2);
    auto avg_hi_sh = _mm_srli_si128(avg_hi, 2);

    avg_lo = _mm_avg_epu16(avg_lo, avg_lo_sh);
    avg_hi = _mm_avg_epu16(avg_hi, avg_hi_sh);

    avg_lo = _mm_shufflelo_epi16(avg_lo, _MM_SHUFFLE(3, 3, 2, 0));
    avg_lo = _mm_shufflehi_epi16(avg_lo, _MM_SHUFFLE(3, 3, 2, 0));
    avg_lo = _mm_shuffle_epi32(avg_lo, _MM_SHUFFLE(3, 3, 2, 0));

    avg_hi = _mm_shufflelo_epi16(avg_hi, _MM_SHUFFLE(3, 3, 2, 0));
    avg_hi = _mm_shufflehi_epi16(avg_hi, _MM_SHUFFLE(3, 3, 2, 0));
    avg_hi = _mm_shuffle_epi32(avg_hi, _MM_SHUFFLE(2, 0, 3, 3));

    return simd_blend_epi8<CPU_SSE2>(_mm_set_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0, 0), avg_hi, avg_lo);
}


template <MaskMode mode, Processor merge_c>
void merge16_t_stacked_sse2(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
                                                   const Byte *pMask, ptrdiff_t nMaskPitch, int nWidth, int nHeight)
{
    int wMod8 = (nWidth / 8) * 8;
    auto pDst_s = pDst;
    auto pSrc1_s = pSrc1;
    auto pMask_s = pMask;

    auto pDstLsb = pDst + nDstPitch * nHeight / 2;
    auto pSrc1Lsb = pSrc1 + nSrc1Pitch * nHeight / 2;
    auto pMaskLsb = pMask + nMaskPitch * nHeight / (mode == MASK420 ? 1 : 2);

    auto zero = _mm_setzero_si128();
    #pragma warning(disable: 4309)
    auto ffff = _mm_set1_epi16(0xFFFF);
    #pragma warning(default: 4309)
    auto ff = _mm_set1_epi16(0x00FF);

    for ( int j = 0; j < nHeight / 2; ++j ) {
        for ( int i = 0; i < wMod8; i+=8 ) {
            auto dst = get_value_stacked_sse2(pDst, pDstLsb, i);
            auto src = get_value_stacked_sse2(pSrc1, pSrc1Lsb, i);
            __m128i mask;

            if (mode == MASK420) {
                mask = get_mask_420_stacked_sse2(pMask, pMaskLsb, nMaskPitch, i);
            } else {
                mask = get_mask_stacked_sse2(pMask, pMaskLsb, nMaskPitch, i);
            }

            auto dst_lo = _mm_unpacklo_epi16(dst, zero);
            auto dst_hi = _mm_unpackhi_epi16(dst, zero);

            auto src_lo = _mm_unpacklo_epi16(src, zero);
            auto src_hi = _mm_unpackhi_epi16(src, zero);

            auto mask_lo = _mm_unpacklo_epi16(mask, zero);
            auto mask_hi = _mm_unpackhi_epi16(mask, zero);

            auto diff_lo = _mm_sub_epi32(src_lo, dst_lo);
            auto diff_hi = _mm_sub_epi32(src_hi, dst_hi);

            auto smask_lo = _mm_srai_epi32(mask_lo, 1);
            auto smask_hi = _mm_srai_epi32(mask_hi, 1);

            auto lerp_lo = simd_mullo_epi32<CPU_SSE2>(diff_lo, smask_lo);
            auto lerp_hi = simd_mullo_epi32<CPU_SSE2>(diff_hi, smask_hi);

            lerp_lo = _mm_srai_epi32(lerp_lo, 15);
            lerp_hi = _mm_srai_epi32(lerp_hi, 15);

            auto result_lo = _mm_add_epi32(dst_lo, lerp_lo);
            auto result_hi = _mm_add_epi32(dst_hi, lerp_hi);

            auto result = _mm_packus_epi32(result_lo, result_hi);

            auto mask_FFFF = _mm_cmpeq_epi16(mask, ffff);
            auto mask_zero = _mm_cmpeq_epi16(mask, zero);

            result = simd_blend_epi8<CPU_SSE2>(mask_FFFF, src, result);
            result = simd_blend_epi8<CPU_SSE2>(mask_zero, dst, result);

            auto result_lsb = _mm_and_si128(result, ff);
            auto result_msb = _mm_srli_epi16(result, 8);

            result_lsb = _mm_packus_epi16(result_lsb, zero);
            result_msb = _mm_packus_epi16(result_msb, zero);

            _mm_storel_epi64(reinterpret_cast<__m128i*>(pDst+i), result_msb);
            _mm_storel_epi64(reinterpret_cast<__m128i*>(pDstLsb+i), result_lsb);
        }

        pDst += nDstPitch;
        pDstLsb += nDstPitch;
        pSrc1 += nSrc1Pitch;
        pSrc1Lsb += nSrc1Pitch;

        if (mode == MASK420) {
            pMask += nMaskPitch * 2;
            pMaskLsb += nMaskPitch * 2;
        } else {
            pMask += nMaskPitch;
            pMaskLsb += nMaskPitch;
        }
    }

    if (nWidth > wMod8) {
        merge_c(pDst_s + wMod8, nDstPitch, pSrc1_s + wMod8, nSrc1Pitch, pMask_s + wMod8, nMaskPitch, nWidth-wMod8, nHeight);
    }
}

Processor *merge16_c_stacked = &merge16_t_stacked_c<MASK444>;
Processor *merge16_luma_420_c_stacked = &merge16_t_stacked_c<MASK420>;

Processor *merge16_sse2_stacked = merge16_t_stacked_sse2<MASK444, merge16_t_stacked_c<MASK444>>;
Processor *merge16_luma_420_sse2_stacked = merge16_t_stacked_sse2<MASK420, merge16_t_stacked_c<MASK420>>;

} } } }