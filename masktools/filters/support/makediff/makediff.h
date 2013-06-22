#ifndef __Mt_MakeDiff_H__
#define __Mt_MakeDiff_H__

#include "../../../common/base/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Support { namespace MakeDiff {

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight);

Processor makediff_c;
extern Processor *makediff_sse2;
extern Processor *makediff_asse2;

class MakeDiff : public MaskTools::Filter
{

   ProcessorList<Processor> processors;

protected:

   virtual void process(int n, const Plane<Byte> &dst, int nPlane)
   {
      UNUSED(n);
      processors.best_processor( constraints[nPlane] )( dst, dst.pitch(), frames[0].plane(nPlane), frames[0].plane(nPlane).pitch(), dst.width(), dst.height() );
   }

public:
   MakeDiff(const Parameters &parameters) : MaskTools::Filter( parameters, FilterProcessingType::INPLACE )
   {
      /* add the processors */
      processors.push_back(Filtering::Processor<Processor>(&makediff_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
      processors.push_back(Filtering::Processor<Processor>(makediff_sse2, Constraint(CPU_SSE2, 1, 1, 1, 1), 1));
      processors.push_back(Filtering::Processor<Processor>(makediff_asse2, Constraint(CPU_SSE2, 1, 1, 16, 16), 2));
   }

   InputConfiguration &input_configuration() const { return InPlaceTwoFrame(); }

   static Signature filter_signature()
   {
      Signature signature = "mt_makediff";

      signature.add(Parameter(TYPE_CLIP, ""));
      signature.add(Parameter(TYPE_CLIP, ""));

      return add_defaults( signature );
   }

};

} } } } }

#endif