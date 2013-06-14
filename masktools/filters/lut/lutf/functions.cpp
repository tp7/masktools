#include "filter.h"
#include "../functions.h"

using namespace Filtering;

template<class T>
static void frame_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, const Byte *pLut, int nWidth, int nHeight)
{
   T new_value( "" );

   new_value.reset();

   for ( int j = 0; j < nHeight; j++, pDst += nDstPitch )
      for ( int i = 0; i < nWidth; i++ )
         new_value.add( pDst[i] );

   const Byte *lut = &pLut[ ( new_value.finalize() << 8 ) ];

   pDst -= nDstPitch * nHeight;

   for ( int j = 0; j < nHeight; j++, pSrc += nSrcPitch, pDst += nDstPitch )
      for ( int i = 0; i < nWidth; i++ )
         pDst[i] = lut[ pSrc[i] ];
}

namespace Filtering { namespace MaskTools { namespace Filters { namespace Lut { namespace Frame {

Processor *processors_array[NUM_MODES] = MPROCESSOR_SINGLE( frame_c );

} } } } }

