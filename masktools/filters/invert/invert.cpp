#include "invert.h"
#include "../../common/simd.h"

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

void invert_sse2(Byte *pDst, ptrdiff_t nDstPitch, int nWidth, int nHeight)
{
    auto fff = _mm_set1_epi32(0xFFFFFFFF);
    for ( int j = 0; j < nHeight; j++ ) {
        for ( int i = 0; i < nWidth; i+=16 ) {
            auto src = simd_load_si128<MemoryMode::SSE2_ALIGNED>(pDst+i);
            auto result = _mm_xor_si128(src, fff);
            simd_store_si128<MemoryMode::SSE2_ALIGNED>(pDst+i, result);
        }
        pDst += nDstPitch;
    }
}


} } } }
