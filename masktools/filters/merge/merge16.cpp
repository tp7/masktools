#include "merge16.h"
#include "../../common/simd.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Merge16 {

/* Common */

enum MaskMode {
    MASK420,
    MASK444
};

MT_FORCEINLINE static Word merge_core_c(Word dst, Word src, Word mask) {
    if (mask == 0) {
        return dst;
    } else if (mask == 65535) {
        return src;
    } else {
        return dst +(((src - dst) * (mask >> 1)) >> 15);
    }
}

template<CpuFlags flags>
MT_FORCEINLINE static __m128i merge_core_simd(const __m128i &dst, const __m128i &src, const __m128i &mask, const __m128i &ffff, const __m128i &zero) {
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

    auto lerp_lo = simd_mullo_epi32<flags>(diff_lo, smask_lo);
    auto lerp_hi = simd_mullo_epi32<flags>(diff_hi, smask_hi);

    lerp_lo = _mm_srai_epi32(lerp_lo, 15);
    lerp_hi = _mm_srai_epi32(lerp_hi, 15);

    auto result_lo = _mm_add_epi32(dst_lo, lerp_lo);
    auto result_hi = _mm_add_epi32(dst_hi, lerp_hi);

    auto result = _mm_packus_epi32(result_lo, result_hi);

    auto mask_FFFF = _mm_cmpeq_epi16(mask, ffff);
    auto mask_zero = _mm_cmpeq_epi16(mask, zero);

    result = simd_blend_epi8<flags>(mask_FFFF, src, result);
    result = simd_blend_epi8<flags>(mask_zero, dst, result);

    return result;
}

/* Stacked */

MT_FORCEINLINE static Word get_value_stacked_c(const Byte *pMsb, const Byte *pLsb, int x) {
    return (Word(pMsb[x]) << 8) + pLsb[x];
}

MT_FORCEINLINE static Word get_mask_stacked_c(const Byte *pMsb, const Byte *pLsb, int, int x) {
    return get_value_stacked_c(pMsb, pLsb, x);
}

MT_FORCEINLINE static Word get_mask_420_stacked_c(const Byte *pMsb, const Byte *pLsb, int pitch, int x) {
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

            Word output = merge_core_c(dst, src, mask);

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

MT_FORCEINLINE static __m128i get_value_stacked_simd(const Byte *pMsb, const Byte *pLsb, int x) {
    auto msb = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(pMsb+x));
    auto lsb = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(pLsb+x));
    return _mm_unpacklo_epi8(lsb, msb);
}

MT_FORCEINLINE static __m128i get_mask_stacked_simd(const Byte *pMsb, const Byte *pLsb, int, int x) {
    return get_value_stacked_simd(pMsb, pLsb, x);
}

template <CpuFlags flags>
MT_FORCEINLINE static __m128i get_mask_420_stacked_simd(const Byte *pMsb, const Byte *pLsb, int pitch, int x) {
    x = x*2;

    auto row1_lo = get_value_stacked_simd(pMsb, pLsb, x);
    auto row1_hi = get_value_stacked_simd(pMsb, pLsb, x+8);
    auto row2_lo = get_value_stacked_simd(pMsb+pitch, pLsb+pitch, x);
    auto row2_hi = get_value_stacked_simd(pMsb+pitch, pLsb+pitch, x+8);

    auto avg_lo = _mm_avg_epu16(row1_lo, row2_lo);
    auto avg_hi = _mm_avg_epu16(row1_hi, row2_hi);

    auto avg_lo_sh = _mm_srli_si128(avg_lo, 2);
    auto avg_hi_sh = _mm_srli_si128(avg_hi, 2);

    avg_lo = _mm_avg_epu16(avg_lo, avg_lo_sh);
    avg_hi = _mm_avg_epu16(avg_hi, avg_hi_sh);

    if (flags >= CPU_SSSE3) {
        avg_lo = _mm_shuffle_epi8(avg_lo, _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 13, 12, 9, 8, 5, 4, 1, 0));
        avg_hi = _mm_shuffle_epi8(avg_hi, _mm_set_epi8(13, 12, 9, 8, 5, 4, 1, 0, 0, 0, 0, 0, 0, 0 ,0 ,0));
    } else {
        avg_lo = _mm_shufflelo_epi16(avg_lo, _MM_SHUFFLE(3, 3, 2, 0));
        avg_lo = _mm_shufflehi_epi16(avg_lo, _MM_SHUFFLE(3, 3, 2, 0));
        avg_lo = _mm_shuffle_epi32(avg_lo, _MM_SHUFFLE(3, 3, 2, 0));

        avg_hi = _mm_shufflelo_epi16(avg_hi, _MM_SHUFFLE(3, 3, 2, 0));
        avg_hi = _mm_shufflehi_epi16(avg_hi, _MM_SHUFFLE(3, 3, 2, 0));
        avg_hi = _mm_shuffle_epi32(avg_hi, _MM_SHUFFLE(2, 0, 3, 3));
    }

    return simd_blend_epi8<flags>(_mm_set_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0, 0), avg_hi, avg_lo);
}


template <CpuFlags flags, MaskMode mode, Processor merge_c>
void merge16_t_stacked_simd(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
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
            auto dst = get_value_stacked_simd(pDst, pDstLsb, i);
            auto src = get_value_stacked_simd(pSrc1, pSrc1Lsb, i);
            __m128i mask;

            if (mode == MASK420) {
                mask = get_mask_420_stacked_simd<flags>(pMask, pMaskLsb, nMaskPitch, i);
            } else {
                mask = get_mask_stacked_simd(pMask, pMaskLsb, nMaskPitch, i);
            }

            auto result = merge_core_simd<flags>(dst, src, mask, ffff, zero);

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

/* Interleaved */

MT_FORCEINLINE static Word get_mask_420_interleaved_c(const Byte *ptr, int pitch, int x) {
    x = x*2;

    return (((reinterpret_cast<const Word*>(ptr)[x] + reinterpret_cast<const Word*>(ptr+pitch)[x] + 1) >> 1) + 
        ((reinterpret_cast<const Word*>(ptr)[x+1] +  reinterpret_cast<const Word*>(ptr+pitch)[x+1] + 1) >> 1) + 1) >> 1;
}


template<MaskMode mode>
void merge16_t_interleaved_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
                         const Byte *pMask, ptrdiff_t nMaskPitch, int nWidth, int nHeight)
{
    for ( int y = 0; y < nHeight; ++y )
    {
        for ( int x = 0; x < nWidth / 2; ++x ) {
            Word dst = reinterpret_cast<const Word*>(pDst)[x];
            Word src = reinterpret_cast<const Word*>(pSrc1)[x];
            Word mask;

            if (mode == MASK420) {
                mask = get_mask_420_interleaved_c(pMask, nMaskPitch, x);
            } else {
                mask = reinterpret_cast<const Word*>(pMask)[x];
            }

            Word output = merge_core_c(dst, src, mask);

            reinterpret_cast<Word*>(pDst)[x] = output;
        }
        pDst += nDstPitch;
        pSrc1 += nSrc1Pitch;

        if (mode == MASK420) {
            pMask += nMaskPitch * 2;
        } else {
            pMask += nMaskPitch;
        }
    }
}


template <CpuFlags flags>
MT_FORCEINLINE static __m128i get_mask_420_interleaved_simd(const Byte *ptr, int pitch, int x) {
    x = x*2;

    auto row1_lo = simd_loadu_epi128(reinterpret_cast<const __m128i*>(ptr+x));
    auto row1_hi = simd_loadu_epi128(reinterpret_cast<const __m128i*>(ptr+x+16));
    auto row2_lo = simd_loadu_epi128(reinterpret_cast<const __m128i*>(ptr+pitch+x));
    auto row2_hi = simd_loadu_epi128(reinterpret_cast<const __m128i*>(ptr+pitch+x+16));

    auto avg_lo = _mm_avg_epu16(row1_lo, row2_lo);
    auto avg_hi = _mm_avg_epu16(row1_hi, row2_hi);

    auto avg_lo_sh = _mm_srli_si128(avg_lo, 2);
    auto avg_hi_sh = _mm_srli_si128(avg_hi, 2);

    avg_lo = _mm_avg_epu16(avg_lo, avg_lo_sh);
    avg_hi = _mm_avg_epu16(avg_hi, avg_hi_sh);

    if (flags >= CPU_SSSE3) {
        avg_lo = _mm_shuffle_epi8(avg_lo, _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 13, 12, 9, 8, 5, 4, 1, 0));
        avg_hi = _mm_shuffle_epi8(avg_hi, _mm_set_epi8(13, 12, 9, 8, 5, 4, 1, 0, 0, 0, 0, 0, 0, 0 ,0 ,0));
    } else {
        avg_lo = _mm_shufflelo_epi16(avg_lo, _MM_SHUFFLE(3, 3, 2, 0));
        avg_lo = _mm_shufflehi_epi16(avg_lo, _MM_SHUFFLE(3, 3, 2, 0));
        avg_lo = _mm_shuffle_epi32(avg_lo, _MM_SHUFFLE(3, 3, 2, 0));

        avg_hi = _mm_shufflelo_epi16(avg_hi, _MM_SHUFFLE(3, 3, 2, 0));
        avg_hi = _mm_shufflehi_epi16(avg_hi, _MM_SHUFFLE(3, 3, 2, 0));
        avg_hi = _mm_shuffle_epi32(avg_hi, _MM_SHUFFLE(2, 0, 3, 3));
    }

    return simd_blend_epi8<flags>(_mm_set_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0, 0), avg_hi, avg_lo);
}


template <CpuFlags flags, MaskMode mode, Processor merge_c>
void merge16_t_interleaved_simd(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
                            const Byte *pMask, ptrdiff_t nMaskPitch, int nWidth, int nHeight)
{
    int wMod16 = (nWidth / 16) * 16;
    auto pDst_s = pDst;
    auto pSrc1_s = pSrc1;
    auto pMask_s = pMask;

    auto zero = _mm_setzero_si128();
#pragma warning(disable: 4309)
    auto ffff = _mm_set1_epi16(0xFFFF);
#pragma warning(default: 4309)

    for ( int j = 0; j < nHeight; ++j ) {
        for ( int i = 0; i < wMod16; i+=16 ) {
            auto dst = simd_loadu_epi128(reinterpret_cast<const __m128i*>(pDst+i));
            auto src = simd_loadu_epi128(reinterpret_cast<const __m128i*>(pSrc1+i));
            __m128i mask;

            if (mode == MASK420) {
                mask = get_mask_420_interleaved_sse2<flags>(pMask, nMaskPitch, i);
            } else {
                mask = simd_loadu_epi128(reinterpret_cast<const __m128i*>(pMask+i));
            }

            auto result = merge_core_simd<flags>(dst, src, mask, ffff, zero);

            simd_storeu_epi128(reinterpret_cast<__m128i*>(pDst+i), result);
        }

        pDst += nDstPitch;
        pSrc1 += nSrc1Pitch;

        if (mode == MASK420) {
            pMask += nMaskPitch * 2;
        } else {
            pMask += nMaskPitch;
        }
    }

    if (nWidth > wMod16) {
        merge_c(pDst_s + wMod16, nDstPitch, pSrc1_s + wMod16, nSrc1Pitch, pMask_s + wMod16, nMaskPitch, nWidth-wMod16, nHeight);
    }
}


Processor *merge16_c_stacked = &merge16_t_stacked_c<MASK444>;
Processor *merge16_luma_420_c_stacked = &merge16_t_stacked_c<MASK420>;

Processor *merge16_sse2_stacked = merge16_t_stacked_simd<CPU_SSE2, MASK444, merge16_t_stacked_c<MASK444>>;
Processor *merge16_sse4_1_stacked = merge16_t_stacked_simd<CPU_SSE4_1, MASK444, merge16_t_stacked_c<MASK444>>;

Processor *merge16_luma_420_sse2_stacked = merge16_t_stacked_simd<CPU_SSE2, MASK420, merge16_t_stacked_c<MASK420>>;
Processor *merge16_luma_420_ssse3_stacked = merge16_t_stacked_simd<CPU_SSSE3, MASK420, merge16_t_stacked_c<MASK420>>;
Processor *merge16_luma_420_sse4_1_stacked = merge16_t_stacked_simd<CPU_SSE4_1, MASK420, merge16_t_stacked_c<MASK420>>;


Processor *merge16_c_interleaved = &merge16_t_interleaved_c<MASK444>;
Processor *merge16_luma_420_c_interleaved = &merge16_t_interleaved_c<MASK420>;

Processor *merge16_sse2_interleaved = merge16_t_interleaved_simd<CPU_SSE2, MASK444, merge16_t_interleaved_c<MASK444>>;
Processor *merge16_sse4_1_interleaved = merge16_t_interleaved_simd<CPU_SSE4_1, MASK444, merge16_t_interleaved_c<MASK444>>;

Processor *merge16_luma_420_sse2_interleaved = merge16_t_interleaved_simd<CPU_SSE2, MASK420, merge16_t_interleaved_c<MASK420>>;
Processor *merge16_luma_420_ssse3_interleaved = merge16_t_interleaved_simd<CPU_SSSE3, MASK420, merge16_t_interleaved_c<MASK420>>;
Processor *merge16_luma_420_sse4_1_interleaved = merge16_t_interleaved_simd<CPU_SSE4_1, MASK420, merge16_t_interleaved_c<MASK420>>;

} } } }