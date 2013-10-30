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

MT_FORCEINLINE __m128i merge_sse2_core(Byte *pDst, const Byte *pSrc, const __m128i& mask_lo, const __m128i& mask_hi, 
                                       const __m128i& v128, const __m128i& v256, const __m128i& zero) {
    auto dst_t1 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(pDst));
    auto dst_t2 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(pDst+8));
    auto unpacked_dst_t1 = _mm_unpacklo_epi8(dst_t1, zero);
    auto unpacked_dst_t2 = _mm_unpacklo_epi8(dst_t2, zero);

    auto src_t1 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(pSrc));
    auto src_t2 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(pSrc+8));
    auto unpacked_src1_t1 = _mm_unpacklo_epi8(src_t1, zero);
    auto unpacked_src1_t2 = _mm_unpacklo_epi8(src_t2, zero);

    auto temp1_t1 = _mm_mullo_epi16(_mm_sub_epi16(v256, mask_lo), unpacked_dst_t1); //(256 - pSrc2[x]) * pDst[x]
    auto temp1_t2 = _mm_mullo_epi16(_mm_sub_epi16(v256, mask_hi), unpacked_dst_t2);

    auto temp2_t1 = _mm_mullo_epi16(mask_lo, unpacked_src1_t1); // pSrc2[x] * pSrc1[x]
    auto temp2_t2 = _mm_mullo_epi16(mask_hi, unpacked_src1_t2);

    temp1_t1 = _mm_add_epi16(temp1_t1, temp2_t1);
    temp1_t2 = _mm_add_epi16(temp1_t2, temp2_t2);

    temp1_t1 = _mm_add_epi16(temp1_t1, v128);
    temp1_t2 = _mm_add_epi16(temp1_t2, v128);

    auto result1 = _mm_srli_epi16(temp1_t1, 8);
    auto result2 = _mm_srli_epi16(temp1_t2, 8);

    return _mm_packus_epi16(result1, result2);
}

template <decltype(simd_store_epi128) store>
void merge_sse2_t(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
                                                   const Byte *pMask, ptrdiff_t nSrc2Pitch, int nWidth, int nHeight)
{
    int wMod16 = (nWidth / 16) * 16;
    auto pDst_s = pDst;
    auto pSrc1_s = pSrc1;
    auto pMask_s = pMask;
    auto v256 = _mm_set1_epi16(0x0100);
    auto v128 = _mm_set1_epi16(0x0080);
    auto zero = _mm_setzero_si128();
    for ( int j = 0; j < nHeight; ++j ) {
        for ( int i = 0; i < wMod16; i+=16 ) {
            auto src2_t1 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(pMask+i));
            auto src2_t2 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(pMask+8+i));
            auto mask_t1 = _mm_unpacklo_epi8(src2_t1, zero);
            auto mask_t2 = _mm_unpacklo_epi8(src2_t2, zero);

            auto result = merge_sse2_core(pDst+i, pSrc1+i, mask_t1, mask_t2, v128, v256, zero);

            store(reinterpret_cast<__m128i*>(pDst+i), result);
        }
        pDst += nDstPitch;
        pSrc1 += nSrc1Pitch;
        pMask += nSrc2Pitch;
    }

    if (nWidth > wMod16) {
        merge_c(pDst_s + wMod16, nDstPitch, pSrc1_s + wMod16, nSrc1Pitch, pMask_s + wMod16, nSrc2Pitch, nWidth-wMod16, nHeight);
    }
}

template <decltype(simd_load_epi128) load, decltype(simd_store_epi128) store>
void merge_luma_420_sse2_t(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
                  const Byte *pMask, ptrdiff_t nSrc2Pitch, int nWidth, int nHeight)
{
    int wMod16 = (nWidth / 16) * 16;
    auto pDst_s = pDst;
    auto pSrc1_s = pSrc1;
    auto mPask_s = pMask;
    auto v256 = _mm_set1_epi16(0x0100);
    auto v255 = _mm_set1_epi16(0x00FF);
    auto v128 = _mm_set1_epi16(0x0080);
    auto zero = _mm_setzero_si128();
    for ( int j = 0; j < nHeight; ++j ) {
        for ( int i = 0; i < wMod16; i+=16 ) {
            _mm_prefetch(reinterpret_cast<const char*>(pMask)+ i*2 + 64, _MM_HINT_T0);
            _mm_prefetch(reinterpret_cast<const char*>(pMask)+ nSrc2Pitch + i*2 + 64, _MM_HINT_T0);
            // preparing mask
            auto src2_row1_t1 = load(reinterpret_cast<const __m128i*>(pMask + i*2));
            auto src2_row1_t2 = load(reinterpret_cast<const __m128i*>(pMask + i*2 + 16));
            auto src2_row2_t1 = load(reinterpret_cast<const __m128i*>(pMask + nSrc2Pitch + i*2));
            auto src2_row2_t2 = load(reinterpret_cast<const __m128i*>(pMask + nSrc2Pitch + i*2 + 16));
            auto avg_t1 = _mm_avg_epu8(src2_row1_t1,src2_row2_t1);
            auto avg_t2 = _mm_avg_epu8(src2_row1_t2,src2_row2_t2);
            auto shifted_t1 = _mm_srli_si128(avg_t1, 1);
            auto shifted_t2 = _mm_srli_si128(avg_t2, 1);
            avg_t1 = _mm_avg_epu8(avg_t1, shifted_t1);
            avg_t2 = _mm_avg_epu8(avg_t2, shifted_t2);
            auto mask_t1 = _mm_and_si128(avg_t1, v255);
            auto mask_t2 = _mm_and_si128(avg_t2, v255);

            auto result = merge_sse2_core(pDst+i, pSrc1+i, mask_t1, mask_t2, v128, v256, zero);
          
            store(reinterpret_cast<__m128i*>(pDst+i), result);
        }
        pDst += nDstPitch;
        pSrc1 += nSrc1Pitch;
        pMask += nSrc2Pitch * 2;
    }
    if (nWidth > wMod16) {
        merge_luma_420_c(pDst_s + wMod16, nDstPitch, pSrc1_s + wMod16, nSrc1Pitch, mPask_s + wMod16*2, nSrc2Pitch, nWidth-wMod16, nHeight);
    }
}

Processor *merge_sse2 = merge_sse2_t<simd_storeu_epi128>;
Processor *merge_asse2 = merge_sse2_t<simd_store_epi128>;
Processor *merge_luma_420_sse2 = merge_luma_420_sse2_t<simd_loadu_epi128, simd_storeu_epi128>;
Processor *merge_luma_420_asse2 = merge_luma_420_sse2_t<simd_load_epi128, simd_store_epi128>;

} } } }