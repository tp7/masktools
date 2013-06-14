#ifndef __Mt_Hysteresis_H__
#define __Mt_Hysteresis_H__

#include "../../../common/base/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Mask { namespace Hysteresis {

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
                        const Byte *pSrc2, ptrdiff_t nSrc2Pitch, Byte *pTemp, int nWidth, int nHeight);

Processor hysteresis_c;

class Filter : public MaskTools::Filter<ChildFilter>
{

   Byte *pbStack;

   void initializeStack()
   {
      if ( !pbStack )
         pbStack = new Byte[nWidth * nHeight];
   }

protected:


   virtual void process(int n, const Plane<Byte> &dst, int nPlane)
   {
      UNUSED(n);
      initializeStack();
      hysteresis_c( dst, dst.pitch(), frames[0].plane(nPlane), frames[0].plane(nPlane).pitch(), frames[1].plane(nPlane), frames[1].plane(nPlane).pitch(), pbStack, dst.width(), dst.height() );
   }

public:
   Filter(const Parameters &parameters) : MaskTools::Filter<ChildFilter>( parameters ), pbStack(NULL) { }
   ~Filter()
   {
      if ( pbStack )
         delete pbStack;
   }

   InputConfiguration &input_configuration() const { return TwoFrame(); }

   static Signature filter_signature()
   {
      Signature signature = "mt_hysteresis";

      signature.add(Parameter(TYPE_CLIP, ""));
      signature.add(Parameter(TYPE_CLIP, ""));

      return add_defaults( signature );
   }
};


} } } } }

#endif