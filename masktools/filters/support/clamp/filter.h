#ifndef __Mt_Clamp_H__
#define __Mt_Clamp_H__

#include "../../../common/base/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Support { namespace Clamp {

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch, const Byte *pSrc2, ptrdiff_t nSrc2Pitch, int nWidth, int nHeight, int nOvershoot, int nUndershoot);

Processor clamp_c;
extern "C" Processor Clamp_clamp8_isse;
extern "C" Processor Clamp_clamp8_3dnow;
extern "C" Processor Clamp_clamp8_sse2;
extern "C" Processor Clamp_clamp8_asse2;

class Filter : public MaskTools::Filter<InPlaceFilter>
{

   int nOvershoot, nUndershoot;
   ProcessorList<Processor> processors;

protected:

   virtual void process(int n, const Plane<Byte> &dst, int nPlane)
   {
      UNUSED(n);
      processors.best_processor( constraints[nPlane] )( dst, dst.pitch(), frames[0].plane(nPlane), frames[0].plane(nPlane).pitch(), frames[1].plane(nPlane), frames[1].plane(nPlane).pitch(), dst.width(), dst.height(), nOvershoot, nUndershoot );
   }

public:
   Filter(const Parameters &parameters) : MaskTools::Filter<InPlaceFilter>( parameters )
   {
      nUndershoot = parameters["undershoot"];
      nOvershoot = parameters["overshoot"];

      /* add the processors */
      processors.push_back(Filtering::Processor<Processor>(&clamp_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
      processors.push_back(Filtering::Processor<Processor>(&Clamp_clamp8_isse, Constraint(CPU_ISSE, 8, 1, 1, 1), 1));
      processors.push_back(Filtering::Processor<Processor>(&Clamp_clamp8_3dnow, Constraint(CPU_3DNOW, 8, 1, 1, 1), 2));
      processors.push_back(Filtering::Processor<Processor>(&Clamp_clamp8_sse2, Constraint(CPU_SSE2, 8, 1, 1, 1), 3));
      processors.push_back(Filtering::Processor<Processor>(&Clamp_clamp8_asse2, Constraint(CPU_SSE2, 8, 1, 16, 16), 4));
   }

   InputConfiguration &input_configuration() const { return InPlaceThreeFrame(); }

   static Signature filter_signature()
   {
      Signature signature = "mt_clamp";

      signature.add(Parameter(TYPE_CLIP, ""));
      signature.add(Parameter(TYPE_CLIP, ""));
      signature.add(Parameter(TYPE_CLIP, ""));
      signature.add(Parameter(0, "overshoot"));
      signature.add(Parameter(0, "undershoot"));

      return add_defaults( signature );
   }

};

} } } } }

#endif
