#ifndef __Mt_Inflate_H__
#define __Mt_Inflate_H__

#include "../morphologic.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic { namespace Inflate {

extern Processor *inflate_c;
extern Processor *inflate_sse2;
extern Processor *inflate_asse2;

class Inflate : public Morphologic::MorphologicFilter
{
public:
   Inflate(const Parameters &parameters) : Morphologic::MorphologicFilter(parameters)
   {
      /* add the processors */
      processors.push_back(Filtering::Processor<Processor>(inflate_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
      processors.push_back(Filtering::Processor<Processor>(inflate_sse2, Constraint(CPU_SSE2, 1, 1, 1, 1), 4));
      processors.push_back(Filtering::Processor<Processor>(inflate_asse2, Constraint(CPU_SSE2, 1, 1, 16, 16), 4));
   }

   static Signature Inflate::filter_signature()
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