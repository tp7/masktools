#include "filter.h"
#include "../functions.h"

using namespace Filtering;

template<class T>
static void custom_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, const Byte *pLut, const int *pCoordinates, int nCoordinates, int nWidth, int nHeight, const String &mode)
{
   T new_value( mode );
   for ( int j = 0; j < nHeight; j++ )
   {
      for ( int i = 0; i < nWidth; i++ )
      {
         new_value.reset();
         for ( int k = 0; k < nCoordinates; k+=2 )
         {
            int x = pCoordinates[k] + i;
            int y = pCoordinates[k+1] + j;

            if ( x < 0 ) x = 0;
            if ( x >= nWidth ) x = nWidth - 1;
            if ( y < 0 ) y = 0;
            if ( y >= nHeight ) y = nHeight - 1;

            new_value.add( pLut[( pSrc[i] << 8 ) + pSrc[x + ( y - j ) * nSrcPitch]] );
         }
         pDst[i] = new_value.finalize();
      }
      pSrc += nSrcPitch;
      pDst += nDstPitch;
   }
}

namespace Filtering { namespace MaskTools { namespace Filters { namespace Lut { namespace Spatial {

Processor *processors_array[NUM_MODES] = MPROCESSOR_SINGLE( custom_c );

} } } } }

