#include "filter.h"

using namespace Filtering;

typedef std::pair<int, int> Coordinates;
typedef std::list<Coordinates> CoordinatesList;

static void expandMask(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
                       const Byte *pSrc2, ptrdiff_t nSrc2Pitch, Byte *pTemp, int x, int y, int nWidth, int nHeight)
{
   CoordinatesList coordinates;

   UNUSED(nSrc1Pitch);
   UNUSED(pSrc1);

   pTemp[0] = 255;
   pDst[0]  = 255;

   coordinates.push_back(Coordinates(0, 0));

   while ( !coordinates.empty() )
   {
      /* pop last coordinates */
      Coordinates current = coordinates.back();
      coordinates.pop_back();

      /* check surrounding positions */
      int x_min = current.first  == -x ? current.first : current.first - 1;
      int x_max = current.first  == nWidth - x - 1 ? current.first + 1 : current.first + 2;
      int y_min = current.second == -y ? current.second : current.second - 1;
      int y_max = current.second == nHeight - y - 1 ? current.second + 1 : current.second + 2;

      for ( int j = y_min; j < y_max; j++ )
      {
         for ( int i = x_min; i < x_max; i++ )
         {
            if ( !pTemp[i + j * nWidth] && pSrc2[i + j * nSrc2Pitch] )
            {
               coordinates.push_back(Coordinates(i, j));
               pTemp[i + j * nWidth] = 255;
               pDst[i + j * nDstPitch] = 255;
            }
         }
      }
   }
}

void Filtering::MaskTools::Filters::Mask::Hysteresis::hysteresis_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
                                                                   const Byte *pSrc2, ptrdiff_t nSrc2Pitch, Byte *pTemp, int nWidth, int nHeight)
{

   memset(pDst , 0, nDstPitch * nHeight * sizeof(Byte));
   memset(pTemp, 0, nWidth    * nHeight * sizeof(Byte));

   for ( int y = 0; y < nHeight; y++ )
   {
      for ( int x = 0; x < nWidth; x++ )
      {
         if ( !pTemp[x] && pSrc1[x] && pSrc2[x] )
            expandMask(&pDst[x], nDstPitch, &pSrc1[x], nSrc1Pitch, &pSrc2[x], nSrc2Pitch, &pTemp[x], x, y, nWidth, nHeight);
      }
      pTemp += nWidth;
      pSrc1 += nSrc1Pitch;
      pSrc2 += nSrc2Pitch;
      pDst  += nDstPitch;
   }
}