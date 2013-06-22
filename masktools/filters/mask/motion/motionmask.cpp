#include "motionmask.h"
#include "../../../common/simd.h"
#include "../../../../common/functions/functions.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Mask { namespace Motion {

static unsigned int sad_c_op(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight)
{
   unsigned int nSad = 0;
	for ( int y = 0; y < nHeight; y++ ) 
	{
		for (int x = 0; x < nWidth; x++ )
         nSad += abs(pDst[x] - pSrc[x]);
      pDst += nDstPitch;
      pSrc += nSrcPitch;
	}
   return nSad;
}

static void mask_c_op(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nLowThreshold, int nHighThreshold, int nWidth, int nHeight)
{
	for ( int y = 0; y < nHeight; y++ ) 
	{
		for (int x = 0; x < nWidth; x++ )
         pDst[x] = threshold<Byte, int>( abs<int>( pDst[x] - pSrc[x] ), nLowThreshold, nHighThreshold );
      pDst += nDstPitch;
      pSrc += nSrcPitch;
   }
}


template <decltype(simd_load_epi128) load>
static unsigned int sad_sse2_op(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight)
{
    int wMod32 = (nWidth / 32) * 32;
    auto pDst2 = pDst;
    auto pSrc2 = pSrc;
    __m128i acc = _mm_setzero_si128();
    for ( int j = 0; j < nHeight; ++j ) {
        for ( int i = 0; i < wMod32; i+=32 ) {
            auto dst1 = load(reinterpret_cast<const __m128i*>(pDst+i));
            auto dst2 = load(reinterpret_cast<const __m128i*>(pDst+i+16));
            auto src1 = load(reinterpret_cast<const __m128i*>(pSrc+i));
            auto src2 = load(reinterpret_cast<const __m128i*>(pSrc+i+16));

            auto sad1 = _mm_sad_epu8(dst1, src1);
            auto sad2 = _mm_sad_epu8(dst2, src2);

            acc = _mm_add_epi32(acc, sad1);
            acc = _mm_add_epi32(acc, sad2);
        }
        pDst += nDstPitch;
        pSrc += nSrcPitch;
    }
    
    auto idk = _mm_castps_si128(_mm_movehl_ps(_mm_setzero_ps(), _mm_castsi128_ps(acc)));
    auto sum = _mm_add_epi32(acc, idk);
    auto sad = _mm_cvtsi128_si32(sum);
    if (nWidth > wMod32) {
        sad += sad_c_op(pDst2 + wMod32, nDstPitch, pSrc2 + wMod32, nSrcPitch, nWidth - wMod32, nHeight);
    }
    return sad;
}

template <decltype(simd_load_epi128) load, decltype(simd_store_epi128) store>
static void mask_sse2_op(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nLowThreshold, int nHighThreshold, int nWidth, int nHeight)
{
    int wMod32 = (nWidth / 16) * 16;
    auto pDst2 = pDst;
    auto pSrc2 = pSrc;

    auto v128 = _mm_set1_epi32(0x80808080);
    auto lowThr_v = _mm_set1_epi32((nLowThreshold << 24) | (nLowThreshold << 16) | (nLowThreshold << 8) | nLowThreshold);
    auto highThr_v = _mm_set1_epi32((nHighThreshold << 24) | (nHighThreshold << 16) | (nHighThreshold << 8) | nHighThreshold);
    lowThr_v = _mm_sub_epi8(lowThr_v, v128);
    highThr_v = _mm_sub_epi8(highThr_v, v128);

    for ( int j = 0; j < nHeight; ++j ) {
        for ( int i = 0; i < wMod32; i+=16 ) {
            auto dst1 = load(reinterpret_cast<const __m128i*>(pDst+i));
            auto src1 = load(reinterpret_cast<const __m128i*>(pSrc+i));

            auto greater = _mm_subs_epu8(dst1, src1);
            auto smaller = _mm_subs_epu8(src1, dst1);

            auto sum = _mm_add_epi8(greater, smaller);
            auto acc = _mm_sub_epi8(sum, v128);

            auto gthlow = _mm_cmpgt_epi8(acc, lowThr_v);
            auto gthigh = _mm_cmpgt_epi8(acc, highThr_v);
            
            auto mask = _mm_or_si128(_mm_and_si128(sum, gthlow), gthigh);
            store(reinterpret_cast<__m128i*>(pDst + i), mask);
        }
        pDst += nDstPitch;
        pSrc += nSrcPitch;
    }

    if (nWidth > wMod32) {
        mask_c_op(pDst2 + wMod32, nDstPitch, pSrc2 + wMod32, nSrcPitch, nLowThreshold, nHighThreshold, nWidth - wMod32, nHeight);
    }
}

template <decltype(sad_c_op) sad, decltype(mask_c_op) mask, Functions::Memset memsetPlane>
bool mask_t(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nLowThreshold, int nHighThreshold, int nMotionThreshold, int nSceneChange, int nSceneChangeValue, int nWidth, int nHeight)
{
    bool scenechange = nSceneChange >= 2 ? nSceneChange == 3 : sad(pDst, nDstPitch, pSrc, nSrcPitch, nWidth, nHeight) > (unsigned int)(nMotionThreshold * nWidth * nHeight);

    if ( scenechange )
        memsetPlane(pDst, nDstPitch, nWidth, nHeight, static_cast<Byte>(nSceneChangeValue));
    else
        mask(pDst, nDstPitch, pSrc, nSrcPitch, nLowThreshold, nHighThreshold, nWidth, nHeight);

    return scenechange;
}


Processor *mask_c       = &mask_t<sad_c_op, mask_c_op, Functions::memset_c>;
Processor *mask8_sse2   = &mask_t<sad_sse2_op<simd_loadu_epi128>, mask_sse2_op<simd_loadu_epi128, simd_storeu_epi128>, Functions::memset_c>;
Processor *mask8_asse2  = &mask_t<sad_sse2_op<simd_load_epi128>, mask_sse2_op<simd_load_epi128, simd_store_epi128>, Functions::memset_c>;

} } } } }