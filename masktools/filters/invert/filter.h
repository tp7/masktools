#ifndef __Mt_Invert_H__
#define __Mt_Invert_H__

#include "../../common/base/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Invert {

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, int nWidth, int nHeight);

Processor invert_c;
extern "C" Processor Invert_invert8_mmx;
extern "C" Processor Invert_invert8_isse;
extern "C" Processor Invert_invert8_3dnow;
extern "C" Processor Invert_invert8_sse2;
extern "C" Processor Invert_invert8_asse2;

class Filter : public MaskTools::Filter<InPlaceFilter>
{
   ProcessorList<Processor> processors;

protected:
   virtual void process(int n, const Plane<Byte> &dst, int nPlane)
   {
      UNUSED(n);
      processors.best_processor( constraints[nPlane] )( dst, dst.pitch(), dst.width(), dst.height() );
   }

public:
   Filter(const Parameters &parameters) : MaskTools::Filter<InPlaceFilter>( parameters )
   {
      /* add the processors */
      processors.push_back( Filtering::Processor<Processor>( invert_c, Constraint( CPU_NONE, 1, 1, 1, 1 ), 0 ) );
      processors.push_back( Filtering::Processor<Processor>( Invert_invert8_mmx, Constraint( CPU_MMX, 8, 1, 1, 1 ), 1 ) );
      processors.push_back( Filtering::Processor<Processor>( Invert_invert8_isse, Constraint( CPU_ISSE, 8, 1, 1, 1 ), 2 ) );
      processors.push_back( Filtering::Processor<Processor>( Invert_invert8_3dnow, Constraint( CPU_3DNOW, 8, 1, 1, 1 ), 3 ) );
      processors.push_back( Filtering::Processor<Processor>( Invert_invert8_sse2, Constraint( CPU_SSE2, 8, 1, 1, 1 ), 4 ) );
      processors.push_back( Filtering::Processor<Processor>( Invert_invert8_asse2, Constraint( CPU_SSE2, 8, 1, 16, 16 ), 5 ) );
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