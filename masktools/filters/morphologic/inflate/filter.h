#ifndef __Mt_Inflate_H__
#define __Mt_Inflate_H__

#include "../../../filters/morphologic/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic { namespace Inflate {

extern Processor *inflate_c;
extern "C" Processor Inflate_inflate8_isse;
extern "C" Processor Inflate_inflate8_3dnow;
extern "C" Processor Inflate_inflate8_sse2;
extern "C" Processor Inflate_inflate8_asse2;

class Filter : public Morphologic::Filter
{
public:
   Filter(const Parameters &parameters) : Morphologic::Filter(parameters)
   {
      /* add the processors */
      processors.push_back(Filtering::Processor<Processor>(inflate_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
      processors.push_back(Filtering::Processor<Processor>(Inflate_inflate8_isse, Constraint(CPU_ISSE, 8, 1, 1, 1), 1));
      processors.push_back(Filtering::Processor<Processor>(Inflate_inflate8_3dnow, Constraint(CPU_3DNOW, 8, 1, 1, 1), 2));
      processors.push_back(Filtering::Processor<Processor>(Inflate_inflate8_sse2, Constraint(CPU_SSE2, 8, 1, 1, 1), 3));
      processors.push_back(Filtering::Processor<Processor>(Inflate_inflate8_asse2, Constraint(CPU_SSE2, 8, 1, 16, 16), 4));
   }

   static Signature Filter::filter_signature()
   {
      Signature signature = "mt_inflate";

      signature.add( Parameter(TYPE_CLIP, "") );
      signature.add( Parameter(255, "thY") );
      signature.add( Parameter(255, "thC") );

      return add_defaults( signature );
   }
};

} } } } } // namespace Inflate, Morphologic, Filter, MaskTools, Filtering

#endif