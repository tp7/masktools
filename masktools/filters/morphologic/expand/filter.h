#ifndef __Mt_Expand_H__
#define __Mt_Expand_H__

#include "../../../filters/morphologic/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic { namespace Expand {

extern "C" Processor Expand_square_expand8_isse;
extern "C" Processor Expand_square_expand8_3dnow;
extern "C" Processor Expand_square_expand8_sse2;
extern "C" Processor Expand_square_expand8_asse2;
extern Processor *expand_square_c;

extern "C" Processor Expand_horizontal_expand8_isse;
extern "C" Processor Expand_horizontal_expand8_3dnow;
extern "C" Processor Expand_horizontal_expand8_sse2;
extern "C" Processor Expand_horizontal_expand8_asse2;
extern Processor *expand_horizontal_c;

extern "C" Processor Expand_vertical_expand8_isse;
extern "C" Processor Expand_vertical_expand8_3dnow;
extern "C" Processor Expand_vertical_expand8_sse2;
extern "C" Processor Expand_vertical_expand8_asse2;
extern Processor *expand_vertical_c;

extern "C" Processor Expand_both_expand8_isse;
extern "C" Processor Expand_both_expand8_3dnow;
extern "C" Processor Expand_both_expand8_sse2;
extern "C" Processor Expand_both_expand8_asse2;
extern Processor *expand_both_c;

extern Processor *expand_custom_c;

class Filter : public Morphologic::Filter
{
public:
   Filter(const Parameters&parameters) : Morphologic::Filter( parameters )
   {
      /* add the processors */
      if ( parameters["mode"].toString() == "square" )
      {
         processors.push_back(Filtering::Processor<Processor>(expand_square_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
         processors.push_back(Filtering::Processor<Processor>(Expand_square_expand8_isse, Constraint(CPU_ISSE, 8, 1, 1, 1), 1));
         processors.push_back(Filtering::Processor<Processor>(Expand_square_expand8_3dnow, Constraint(CPU_3DNOW, 8, 1, 1, 1), 2));
         processors.push_back(Filtering::Processor<Processor>(Expand_square_expand8_sse2, Constraint(CPU_SSE2, 8, 1, 1, 1), 3));
         processors.push_back(Filtering::Processor<Processor>(Expand_square_expand8_asse2, Constraint(CPU_SSE2, 8, 1, 16, 16), 4));
      }
      else if ( parameters["mode"].toString() == "horizontal" )
      {
         processors.push_back(Filtering::Processor<Processor>(expand_horizontal_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
         processors.push_back(Filtering::Processor<Processor>(Expand_horizontal_expand8_isse, Constraint(CPU_ISSE, 8, 1, 1, 1), 1));
         processors.push_back(Filtering::Processor<Processor>(Expand_horizontal_expand8_3dnow, Constraint(CPU_3DNOW, 8, 1, 1, 1), 2));
         processors.push_back(Filtering::Processor<Processor>(Expand_horizontal_expand8_sse2, Constraint(CPU_SSE2, 8, 1, 1, 1), 3));
         processors.push_back(Filtering::Processor<Processor>(Expand_horizontal_expand8_asse2, Constraint(CPU_SSE2, 8, 1, 16, 16), 4));
      }
      else if ( parameters["mode"].toString() == "vertical" )
      {
         processors.push_back(Filtering::Processor<Processor>(expand_vertical_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
         processors.push_back(Filtering::Processor<Processor>(Expand_vertical_expand8_isse, Constraint(CPU_ISSE, 8, 1, 1, 1), 1));
         processors.push_back(Filtering::Processor<Processor>(Expand_vertical_expand8_3dnow, Constraint(CPU_3DNOW, 8, 1, 1, 1), 2));
         processors.push_back(Filtering::Processor<Processor>(Expand_vertical_expand8_sse2, Constraint(CPU_SSE2, 8, 1, 1, 1), 3));
         processors.push_back(Filtering::Processor<Processor>(Expand_vertical_expand8_asse2, Constraint(CPU_SSE2, 8, 1, 16, 16), 4));
      }
      else if ( parameters["mode"].toString() == "both" )
      {
         processors.push_back(Filtering::Processor<Processor>(expand_both_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
         processors.push_back(Filtering::Processor<Processor>(Expand_both_expand8_isse, Constraint(CPU_ISSE, 8, 1, 1, 1), 1));
         processors.push_back(Filtering::Processor<Processor>(Expand_both_expand8_3dnow, Constraint(CPU_3DNOW, 8, 1, 1, 1), 2));
         processors.push_back(Filtering::Processor<Processor>(Expand_both_expand8_sse2, Constraint(CPU_SSE2, 8, 1, 1, 1), 3));
         processors.push_back(Filtering::Processor<Processor>(Expand_both_expand8_asse2, Constraint(CPU_SSE2, 8, 1, 16, 16), 4));
      }
      else
      {
         processors.push_back(Filtering::Processor<Processor>(expand_custom_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
         FillCoordinates( parameters["mode"].toString() );
      }
   }

   static Signature Filter::filter_signature()
   {
      Signature signature = "mt_expand";

      signature.add( Parameter(TYPE_CLIP, "") );
      signature.add( Parameter(255, "thY") );
      signature.add( Parameter(255, "thC") );
      signature.add( Parameter(String("square"), "mode") );

      return add_defaults( signature );
   }
};

} } } } } // namespace Expand, Morphologic, Filter, MaskTools, Filtering

#endif