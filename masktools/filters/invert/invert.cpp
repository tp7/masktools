#include "invert.h"
#include "../../common/simd.h"
#include <emmintrin.h>

namespace Filtering { namespace MaskTools { namespace Filters { namespace Invert {

void invert_c(Byte *pDst, ptrdiff_t nDstPitch, int nWidth, int nHeight)
{
   for ( int j = 0; j < nHeight; j++ )
   {
      for ( int i = 0; i < nWidth; i++ )
         pDst[i] = 255 - pDst[i];
      pDst += nDstPitch;
   }
}

template<decltype(simd_load_epi128) load, decltype(simd_store_epi128) store>
void invert_sse2_t(Byte *pDst, ptrdiff_t nDstPitch, int nWidth, int nHeight)
{
    auto pDst2 = pDst;
    int width_mod = (nWidth / 16) * 16;
    for ( int j = 0; j < nHeight; j++ ) {
        for ( int i = 0; i < width_mod; i+=16 ) {
            
            auto src = simd_load_epi128(reinterpret_cast<const __m128i*>(pDst+i));
            auto result = _mm_xor_si128(src, _mm_set1_epi32(0xFFFFFFFF));
            simd_store_epi128(reinterpret_cast<__m128i*>(pDst+i), result);
        }
        pDst += nDstPitch;
    }
    if (nWidth > width_mod) {
        invert_c(pDst2 + (nWidth - width_mod), nDstPitch, nWidth - width_mod, nHeight);
    }
}

Processor *invert_sse2 = &invert_sse2_t<simd_loadu_epi128, simd_storeu_epi128>;
Processor *invert_asse2 = &invert_sse2_t<simd_load_epi128, simd_store_epi128>;

} } } }
