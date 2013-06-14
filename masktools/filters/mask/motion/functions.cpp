#include "filter.h"
#include "../../../../common/functions/functions.h"

using namespace Filtering;

typedef unsigned int (Sad)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight);
typedef void (Mask)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nLowThreshold, int nHighThreshold, int nWidth, int nHeight);

static unsigned int sad_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight)
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

static void mask_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nLowThreshold, int nHighThreshold, int nWidth, int nHeight)
{
	for ( int y = 0; y < nHeight; y++ ) 
	{
		for (int x = 0; x < nWidth; x++ )
         pDst[x] = threshold<Byte, int>( abs<int>( pDst[x] - pSrc[x] ), nLowThreshold, nHighThreshold );
      pDst += nDstPitch;
      pSrc += nSrcPitch;
   }
}

template <Sad sad, Mask mask, Functions::Memset memsetPlane>
bool mask_t(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nLowThreshold, int nHighThreshold, int nMotionThreshold, int nSceneChange, int nSceneChangeValue, int nWidth, int nHeight)
{
   bool scenechange = nSceneChange >= 2 ? nSceneChange == 3 : sad(pDst, nDstPitch, pSrc, nSrcPitch, nWidth, nHeight) > (unsigned int)(nMotionThreshold * nWidth * nHeight);

   if ( scenechange )
      memsetPlane(pDst, nDstPitch, nWidth, nHeight, static_cast<Byte>(nSceneChangeValue));
   else
      mask(pDst, nDstPitch, pSrc, nSrcPitch, nLowThreshold, nHighThreshold, nWidth, nHeight);

   return scenechange;
}

extern "C" Sad Motion_sad8_isse;
extern "C" Sad Motion_sad8_sse2;
extern "C" Sad Motion_sad8_asse2;

extern "C" Mask Motion_motion8_mmx;
extern "C" Mask Motion_motion8_isse;
extern "C" Mask Motion_motion8_sse2;
extern "C" Mask Motion_motion8_asse2;

namespace Filtering { namespace MaskTools { namespace Filters { namespace Mask { namespace Motion {

Processor *mask_c       = &mask_t<::sad_c          , ::mask_c            , Functions::memset_c>;
Processor *mask8_mmx    = &mask_t<::sad_c          , Motion_motion8_mmx  , memset8_mmx>;
Processor *mask8_isse   = &mask_t<Motion_sad8_isse , Motion_motion8_isse , memset8_isse>;
Processor *mask8_sse2   = &mask_t<Motion_sad8_sse2 , Motion_motion8_sse2 , memset8_isse>;
Processor *mask8_asse2  = &mask_t<Motion_sad8_asse2, Motion_motion8_asse2, memset8_isse>;

} } } } }