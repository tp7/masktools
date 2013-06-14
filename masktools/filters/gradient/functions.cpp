#include "filter.h"

using namespace Filtering;

typedef int (Distorsion)(const Byte *pSrc, ptrdiff_t nSrcPitch, const Byte *pRef, ptrdiff_t nRefPitch, int nPrecision);

template<int nBlockSizeX, int nBlockSizeY> int sad(const Byte *pSrc, ptrdiff_t nSrcPitch, const Byte *pRef, ptrdiff_t nRefPitch, int nPrecision)
{
   int nSad = 0;

   for ( int y = 0; y < nBlockSizeY; y++, pSrc += nSrcPitch * nPrecision, pRef += nRefPitch * nPrecision )
      for ( int x = 0; x < nBlockSizeX; x++ )
         nSad += abs<int>( pSrc[x * nPrecision] - pRef[x * nPrecision] );

   return nSad;
}

template<Distorsion distorsion, int nBlockSizeX, int nBlockSizeY> void generic_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, const Byte *pRef, ptrdiff_t nRefPitch, int nWidth, int nHeight, int nX, int nY, int nMinimum, int nMaximum, int nPrecision)
{
   int x, y;

   pSrc += nX + nY * nSrcPitch;

   for ( y = 0; y < nHeight - nBlockSizeY * nPrecision + nPrecision; y++, pRef += nRefPitch, pDst += nDstPitch )
   {
      for ( x = 0; x < nWidth - nBlockSizeX * nPrecision + nPrecision; x++ )
         pDst[x] = clip<Byte, int>( (distorsion( pSrc, nSrcPitch, &pRef[x], nRefPitch, nPrecision ) - nMinimum) * 255 / (nMaximum - nMinimum) );

      for ( ; x < nWidth; x++ )
         pDst[x] = pDst[nWidth - nBlockSizeX * nPrecision + nPrecision - 1];
   }
   for ( y = 0; y < nBlockSizeY * nPrecision - nPrecision; y++ )
      memcpy( &pDst[y * nDstPitch], pDst - nDstPitch, nWidth );
}

namespace Filtering { namespace MaskTools { namespace Filters { namespace Gradient {

Processor *sad_c  = &generic_c<sad<16, 16>, 16, 16>;

} } } }
