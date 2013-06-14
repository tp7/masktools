#include "filter.h"

using namespace Filtering;

void Filtering::MaskTools::Filters::Merge::merge_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
                                                   const Byte *pSrc2, ptrdiff_t nSrc2Pitch, int nWidth, int nHeight)
{
   for ( int y = 0; y < nHeight; y++ )
   {
      for ( int x = 0; x < nWidth; x++ )
         pDst[x] = ((256 - int(pSrc2[x])) * pDst[x] + int(pSrc2[x]) * pSrc1[x] + 128) >> 8;
      pDst += nDstPitch;
      pSrc1 += nSrc1Pitch;
      pSrc2 += nSrc2Pitch;
   }
}

void Filtering::MaskTools::Filters::Merge::merge_luma_420_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
                                                        const Byte *pSrc2, ptrdiff_t nSrc2Pitch, int nWidth, int nHeight)
{
   for ( int y = 0; y < nHeight; y++ )
   {
      for ( int x = 0; x < nWidth; x++ )
      {
         const int nMask = (((pSrc2[x*2] + pSrc2[x*2+nSrc2Pitch] + 1) >> 1) + ((pSrc2[x*2+1] + pSrc2[x*2+nSrc2Pitch+1] + 1) >> 1) + 1) >> 1;
         pDst[x] = static_cast<Byte>(((256 - int(nMask)) * pDst[x] + int(nMask) * pSrc1[x] + 128) >> 8);
      }
      pDst += nDstPitch;
      pSrc1 += nSrc1Pitch;
      pSrc2 += nSrc2Pitch * 2;
   }
}