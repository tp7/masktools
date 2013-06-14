#ifndef __Mt_Deflate_H__
#define __Mt_Deflate_H__

#include "../../../filters/morphologic/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic { namespace Deflate {

extern Processor *deflate_c;
extern "C" Processor Deflate_deflate8_isse;
extern "C" Processor Deflate_deflate8_3dnow;
extern "C" Processor Deflate_deflate8_sse2;
extern "C" Processor Deflate_deflate8_asse2;

class Filter : public Morphologic::Filter
{
public:
   Filter(const Parameters &parameters) : Morphologic::Filter(parameters)
   {
      /* add the processors */
      processors.push_back(Filtering::Processor<Processor>(deflate_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
      processors.push_back(Filtering::Processor<Processor>(Deflate_deflate8_isse, Constraint(CPU_ISSE, 8, 1, 1, 1), 1));
      processors.push_back(Filtering::Processor<Processor>(Deflate_deflate8_3dnow, Constraint(CPU_3DNOW, 8, 1, 1, 1), 2));
      processors.push_back(Filtering::Processor<Processor>(Deflate_deflate8_sse2, Constraint(CPU_SSE2, 8, 1, 1, 1), 3));
      processors.push_back(Filtering::Processor<Processor>(Deflate_deflate8_asse2, Constraint(CPU_SSE2, 8, 1, 16, 16), 4));
   }

   static Signature Filter::filter_signature()
   {
      Signature signature = "mt_deflate";

      signature.add( Parameter(TYPE_CLIP, "") );
      signature.add( Parameter(255, "thY") );
      signature.add( Parameter(255, "thC") );

      return add_defaults( signature );
   }
};

} } } } } // namespace Deflate, Morphologic, Filter, MaskTools, Filtering

#endif