#ifndef __Mt_Invert_H__
#define __Mt_Invert_H__

#include "../../common/base/filter.h"
#include <xmmintrin.h>

namespace Filtering { namespace MaskTools { namespace Filters { namespace Invert {

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, int nWidth, int nHeight);

Processor invert_c;
Processor invert_sse2;

class Invert : public MaskTools::Filter
{
   ProcessorList<Processor> processors;

protected:
   virtual void process(int n, const Plane<Byte> &dst, int nPlane)
   {
      UNUSED(n);
      processors.best_processor( constraints[nPlane] )( dst, dst.pitch(), dst.width(), dst.height() );
   }

public:
   Invert(const Parameters &parameters) : MaskTools::Filter( parameters, FilterProcessingType::INPLACE )
   {
      /* add the processors */
      processors.push_back( Filtering::Processor<Processor>( invert_c, Constraint( CPU_NONE, 1, 1, 1, 1 ), 0 ) );
      processors.push_back( Filtering::Processor<Processor>( invert_sse2, Constraint( CPU_SSE2, 16, 1, 16, 16 ), 1 ) );
   }

   InputConfiguration &input_configuration() const { return InPlaceOneFrame(); }

   static Signature filter_signature()
   {
      Signature signature = "mt_invert";

      signature.add( Parameter( TYPE_CLIP, "" ) );

      return add_defaults( signature );
   }
};


} } } } // namespace Invert, Filters, MaskTools, Filtering

#endif