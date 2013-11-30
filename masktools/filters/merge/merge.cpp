#include "merge.h"
#include "../../common/simd.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Merge {

void merge_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
                                                   const Byte *pMask, ptrdiff_t nSrc2Pitch, int nWidth, int nHeight)
{
   for ( int y = 0; y < nHeight; ++y )
   {
      for ( int x = 0; x < nWidth; ++x )
         pDst[x] = ((256 - int(pMask[x])) * pDst[x] + int(pMask[x]) * pSrc1[x] + 128) >> 8;
      pDst += nDstPitch;
      pSrc1 += nSrc1Pitch;
      pMask += nSrc2Pitch;
   }
}

void merge_luma_420_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
                                                        const Byte *pMask, ptrdiff_t nSrc2Pitch, int nWidth, int nHeight)
{
   for ( int y = 0; y < nHeight; ++y )
   {
      for ( int x = 0; x < nWidth; ++x )
      {
         const int nMask = (((pMask[x*2] + pMask[x*2+nSrc2Pitch] + 1) >> 1) + ((pMask[x*2+1] + pMask[x*2+nSrc2Pitch+1] + 1) >> 1) + 1) >> 1;
         pDst[x] = static_cast<Byte>(((256 - int(nMask)) * pDst[x] + int(nMask) * pSrc1[x] + 128) >> 8);
      }
      pDst += nDstPitch;
      pSrc1 += nSrc1Pitch;
      pMask += nSrc2Pitch * 2;
   }
}

template <MemoryMode mem_mode>
MT_FORCEINLINE __m128i merge_sse2_core(Byte *pDst, const Byte *pSrc, const __m128i& mask_lo, const __m128i& mask_hi, 
                                       const __m128i& v128, const __m128i& zero) {
    auto dst = simd_load_si128<mem_mode>(pDst);
    auto dst_lo = _mm_unpacklo_epi8(dst, zero);
    auto dst_hi = _mm_unpackhi_epi8(dst, zero);

    auto src = simd_load_si128<mem_mode>(pSrc);
    auto src1_lo = _mm_unpacklo_epi8(src, zero);
    auto src1_hi = _mm_unpackhi_epi8(src, zero);

    auto dst_lo_sh = _mm_slli_epi16(dst_lo, 8);
    auto diff_lo = _mm_sub_epi16(src1_lo, dst_lo);
    auto tmp1_lo = _mm_mullo_epi16(diff_lo, mask_lo); // (p2-p1)*mask
    auto tmp2_lo = _mm_or_si128(dst_lo_sh, v128);    // p1<<8 + 128 == p1<<8 | 128
    auto result_lo = _mm_add_epi16(tmp1_lo, tmp2_lo);
    result_lo = _mm_srli_epi16(result_lo, 8);

    auto dst_hi_sh = _mm_slli_epi16(dst_hi, 8);
    auto diff_hi = _mm_sub_epi16(src1_hi, dst_hi);
    auto tmp1_hi = _mm_mullo_epi16(diff_hi, mask_hi); // (p2-p1)*mask
    auto tmp2_hi = _mm_or_si128(dst_hi_sh, v128);    // p1<<8 + 128 == p1<<8 | 128
    auto result_hi = _mm_add_epi16(tmp1_hi, tmp2_hi);
    result_hi = _mm_srli_epi16(result_hi, 8);

    return _mm_packus_epi16(result_lo, result_hi);
}

template <MemoryMode mem_mode>
void merge_sse2_t(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
                                                   const Byte *pMask, ptrdiff_t nSrc2Pitch, int nWidth, int nHeight)
{
    int wMod16 = (nWidth / 16) * 16;
    auto pDst_s = pDst;
    auto pSrc1_s = pSrc1;
    auto pMask_s = pMask;
    auto v128 = _mm_set1_epi16(0x0080);
    auto zero = _mm_setzero_si128();
    for ( int j = 0; j < nHeight; ++j ) {
        for ( int i = 0; i < wMod16; i+=16 ) {
            auto src2 = simd_load_si128<mem_mode>(pMask+i);
            auto mask_t1 = _mm_unpacklo_epi8(src2, zero);
            auto mask_t2 = _mm_unpackhi_epi8(src2, zero);

            auto result = merge_sse2_core<mem_mode>(pDst+i, pSrc1+i, mask_t1, mask_t2, v128, zero);

            simd_store_si128<mem_mode>(pDst+i, result);
        }
        pDst += nDstPitch;
        pSrc1 += nSrc1Pitch;
        pMask += nSrc2Pitch;
    }

    if (nWidth > wMod16) {
        merge_c(pDst_s + wMod16, nDstPitch, pSrc1_s + wMod16, nSrc1Pitch, pMask_s + wMod16, nSrc2Pitch, nWidth-wMod16, nHeight);
    }
}

template <MemoryMode mem_mode>
void merge_luma_420_sse2_t(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
                  const Byte *pMask, ptrdiff_t nSrc2Pitch, int nWidth, int nHeight)
{
    int wMod16 = (nWidth / 16) * 16;
    auto pDst_s = pDst;
    auto pSrc1_s = pSrc1;
    auto mPask_s = pMask;
    auto v255 = _mm_set1_epi16(0x00FF);
    auto v128 = _mm_set1_epi16(0x0080);
    auto zero = _mm_setzero_si128();
    for ( int j = 0; j < nHeight; ++j ) {
        for ( int i = 0; i < wMod16; i+=16 ) {
            _mm_prefetch(reinterpret_cast<const char*>(pMask)+ i*2 + 64, _MM_HINT_T0);
            _mm_prefetch(reinterpret_cast<const char*>(pMask)+ nSrc2Pitch + i*2 + 64, _MM_HINT_T0);
            // preparing mask
            auto src2_row1_t1 = simd_load_si128<mem_mode>(pMask + i*2);
            auto src2_row1_t2 = simd_load_si128<mem_mode>(pMask + i*2 + 16);
            auto src2_row2_t1 = simd_load_si128<mem_mode>(pMask + nSrc2Pitch + i*2);
            auto src2_row2_t2 = simd_load_si128<mem_mode>(pMask + nSrc2Pitch + i*2 + 16);
            auto avg_t1 = _mm_avg_epu8(src2_row1_t1,src2_row2_t1);
            auto avg_t2 = _mm_avg_epu8(src2_row1_t2,src2_row2_t2);
            auto shifted_t1 = _mm_srli_si128(avg_t1, 1);
            auto shifted_t2 = _mm_srli_si128(avg_t2, 1);
            avg_t1 = _mm_avg_epu8(avg_t1, shifted_t1);
            avg_t2 = _mm_avg_epu8(avg_t2, shifted_t2);
            auto mask_t1 = _mm_and_si128(avg_t1, v255);
            auto mask_t2 = _mm_and_si128(avg_t2, v255);

            auto result = merge_sse2_core<mem_mode>(pDst+i, pSrc1+i, mask_t1, mask_t2, v128, zero);
          
            simd_store_si128<mem_mode>(pDst+i, result);
        }
        pDst += nDstPitch;
        pSrc1 += nSrc1Pitch;
        pMask += nSrc2Pitch * 2;
    }
    if (nWidth > wMod16) {
        merge_luma_420_c(pDst_s + wMod16, nDstPitch, pSrc1_s + wMod16, nSrc1Pitch, mPask_s + wMod16*2, nSrc2Pitch, nWidth-wMod16, nHeight);
    }
}

Processor *merge_sse2 = merge_sse2_t<MemoryMode::SSE2_UNALIGNED>;
Processor *merge_asse2 = merge_sse2_t<MemoryMode::SSE2_ALIGNED>;
Processor *merge_luma_420_sse2 = merge_luma_420_sse2_t<MemoryMode::SSE2_UNALIGNED>;
Processor *merge_luma_420_asse2 = merge_luma_420_sse2_t<MemoryMode::SSE2_ALIGNED>;

} } } }