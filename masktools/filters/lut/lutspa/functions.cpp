#include "filter.h"

using namespace Filtering;

void Filtering::MaskTools::Filters::Lut::Coordinate::lut_c(Byte *pDst, ptrdiff_t nDstPitch, int nWidth, int nHeight, const Byte lut[65536])
{
   for ( int y = 0; y < nHeight; y++ )
   {
      for ( int x = 0; x < nWidth; x++ )
         pDst[x] = lut[x + y * nWidth];
      pDst += nDstPitch;
   }
}

