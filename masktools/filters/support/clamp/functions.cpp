#include "filter.h"

using namespace Filtering;

void Filtering::MaskTools::Filters::Support::Clamp::clamp_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch, const Byte *pSrc2, ptrdiff_t nSrc2Pitch, int nWidth, int nHeight, int nOvershoot, int nUndershoot)
{
   for ( int y = 0; y < nHeight; y++, pDst += nDstPitch, pSrc1 += nSrc1Pitch, pSrc2 += nSrc2Pitch )
   {
      for ( int x = 0; x < nWidth; x++ )
      {
         pDst[x] = static_cast<Byte>(pDst[x] > pSrc1[x] + nOvershoot ? pSrc1[x] + nOvershoot : pDst[x]);
         pDst[x] = static_cast<Byte>(pDst[x] < pSrc2[x] - nUndershoot ? pSrc2[x] - nUndershoot : pDst[x]);
      }
   }
}