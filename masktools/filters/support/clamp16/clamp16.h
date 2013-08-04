#ifndef __Mt_Clamp16_H__
#define __Mt_Clamp16_H__

#include "../../../common/base/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Support { namespace Clamp16 {

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pUpLimit, ptrdiff_t nUpLimitPitch, const Byte *pLowLimit, ptrdiff_t nLowLimitPitch, int nWidth, int nHeight, int nOvershoot, int nUndershoot);

Processor clamp16_stacked_c;
Processor clamp16_stacked_sse2;

Processor clamp16_interleaved_c;
Processor clamp16_interleaved_sse2;

class Clamp16 : public MaskTools::Filter
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
    Clamp16(const Parameters &parameters) : MaskTools::Filter( parameters, FilterProcessingType::INPLACE )
    {
        nUndershoot = parameters["undershoot"];
        nOvershoot = parameters["overshoot"];

        /* add the processors */
        if (parameters["stacked"].toBool() == true) {
            processors.push_back(Filtering::Processor<Processor>(clamp16_stacked_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
            processors.push_back(Filtering::Processor<Processor>(clamp16_stacked_sse2, Constraint(CPU_SSE2, 1, 1, 1, 1), 1));
        } else {
            processors.push_back(Filtering::Processor<Processor>(clamp16_interleaved_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
            processors.push_back(Filtering::Processor<Processor>(clamp16_interleaved_sse2, Constraint(CPU_SSE2, 1, 1, 1, 1), 1));
        }
    }

   InputConfiguration &input_configuration() const { return InPlaceThreeFrame(); }

   static Signature filter_signature()
   {
      Signature signature = "mt_clamp16";

      signature.add(Parameter(TYPE_CLIP, ""));
      signature.add(Parameter(TYPE_CLIP, ""));
      signature.add(Parameter(TYPE_CLIP, ""));
      signature.add(Parameter(0, "overshoot"));
      signature.add(Parameter(0, "undershoot"));
      signature.add(Parameter(false, "stacked"));

      return add_defaults( signature );
   }

};

} } } } }

#endif
