#ifndef __Mt_Deflate_H__
#define __Mt_Deflate_H__

#include "../morphologic.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic { namespace Deflate {

extern Processor *deflate_c;
extern Processor *deflate_sse2;
extern Processor *deflate_asse2;

class Deflate : public Morphologic::MorphologicFilter
{
public:
   Deflate(const Parameters &parameters) : Morphologic::MorphologicFilter(parameters)
   {
      /* add the processors */
      processors.push_back(Filtering::Processor<Processor>(deflate_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
      processors.push_back(Filtering::Processor<Processor>(deflate_sse2, Constraint(CPU_SSE2, 1, 1, 1, 1), 4));
      processors.push_back(Filtering::Processor<Processor>(deflate_asse2, Constraint(CPU_SSE2, 1, 1, 16, 16), 4));
   }

   static Signature Deflate::filter_signature()
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