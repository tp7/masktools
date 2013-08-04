#ifndef __Mt_MakeDiff16_H__
#define __Mt_MakeDiff16_H__

#include "../../../common/base/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Support { namespace MakeDiff16 {

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight);

Processor makediff16_stacked_c;
Processor makediff16_stacked_sse2;

Processor makediff16_interleaved_c;
Processor makediff16_interleaved_sse2;

class MakeDiff16 : public MaskTools::Filter
{

   ProcessorList<Processor> processors;

protected:

   virtual void process(int n, const Plane<Byte> &dst, int nPlane)
   {
      UNUSED(n);
      processors.best_processor( constraints[nPlane] )( dst, dst.pitch(), frames[0].plane(nPlane), frames[0].plane(nPlane).pitch(), dst.width(), dst.height() );
   }

public:
   MakeDiff16(const Parameters &parameters) : MaskTools::Filter( parameters, FilterProcessingType::INPLACE )
   {
       /* add the processors */
       if (parameters["stacked"].toBool() == true) {
           processors.push_back(Filtering::Processor<Processor>(makediff16_stacked_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
           processors.push_back(Filtering::Processor<Processor>(makediff16_stacked_sse2, Constraint(CPU_SSE2, 1, 1, 1, 1), 1));
       } else {
           processors.push_back(Filtering::Processor<Processor>(makediff16_interleaved_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
           processors.push_back(Filtering::Processor<Processor>(makediff16_interleaved_sse2, Constraint(CPU_SSE2, 1, 1, 1, 1), 1));
       }
   }

   InputConfiguration &input_configuration() const { return InPlaceTwoFrame(); }

   static Signature filter_signature()
   {
      Signature signature = "mt_makediff16";

      signature.add(Parameter(TYPE_CLIP, ""));
      signature.add(Parameter(TYPE_CLIP, ""));
      signature.add(Parameter(false, "stacked"));

      return add_defaults( signature );
   }

};

} } } } }

#endif