#ifndef __Mt_Inpand_H__
#define __Mt_Inpand_H__

#include "../../../filters/morphologic/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic { namespace Inpand {


extern "C" Processor Inpand_square_inpand8_isse;
extern "C" Processor Inpand_square_inpand8_3dnow;
extern "C" Processor Inpand_square_inpand8_sse2;
extern "C" Processor Inpand_square_inpand8_asse2;
extern Processor *inpand_square_c;

extern "C" Processor Inpand_horizontal_inpand8_isse;
extern "C" Processor Inpand_horizontal_inpand8_3dnow;
extern "C" Processor Inpand_horizontal_inpand8_sse2;
extern "C" Processor Inpand_horizontal_inpand8_asse2;
extern Processor *inpand_horizontal_c;

extern "C" Processor Inpand_vertical_inpand8_isse;
extern "C" Processor Inpand_vertical_inpand8_3dnow;
extern "C" Processor Inpand_vertical_inpand8_sse2;
extern "C" Processor Inpand_vertical_inpand8_asse2;
extern Processor *inpand_vertical_c;

extern "C" Processor Inpand_both_inpand8_isse;
extern "C" Processor Inpand_both_inpand8_3dnow;
extern "C" Processor Inpand_both_inpand8_sse2;
extern "C" Processor Inpand_both_inpand8_asse2;
extern Processor *inpand_both_c;

extern Processor *inpand_custom_c;

class Filter : public Morphologic::Filter
{
public:
   Filter(const Parameters&parameters) : Morphologic::Filter( parameters )
   {
      /* add the processors */
      if ( parameters["mode"].toString() == "square" )
      {
         processors.push_back( Filtering::Processor<Processor>( inpand_square_c, Constraint( CPU_NONE, 1, 1, 1, 1 ), 0 ) );
         processors.push_back( Filtering::Processor<Processor>( Inpand_square_inpand8_isse, Constraint(CPU_ISSE, 8, 1, 1, 1), 1));
         processors.push_back( Filtering::Processor<Processor>( Inpand_square_inpand8_3dnow, Constraint(CPU_3DNOW, 8, 1, 1, 1), 2));
         processors.push_back( Filtering::Processor<Processor>( Inpand_square_inpand8_sse2, Constraint(CPU_SSE2, 8, 1, 1, 1), 3));
         processors.push_back( Filtering::Processor<Processor>( Inpand_square_inpand8_asse2, Constraint(CPU_SSE2, 8, 1, 16, 16), 4));
      }
      else if ( parameters["mode"].toString() == "horizontal" )
      {
         processors.push_back( Filtering::Processor<Processor>( inpand_horizontal_c, Constraint( CPU_NONE, 1, 1, 1, 1 ), 0 ) );
         processors.push_back( Filtering::Processor<Processor>( Inpand_horizontal_inpand8_isse, Constraint(CPU_ISSE, 8, 1, 1, 1), 1));
         processors.push_back( Filtering::Processor<Processor>( Inpand_horizontal_inpand8_3dnow, Constraint(CPU_3DNOW, 8, 1, 1, 1), 2));
         processors.push_back( Filtering::Processor<Processor>( Inpand_horizontal_inpand8_sse2, Constraint(CPU_SSE2, 8, 1, 1, 1), 3));
         processors.push_back( Filtering::Processor<Processor>( Inpand_horizontal_inpand8_asse2, Constraint(CPU_SSE2, 8, 1, 16, 16), 4));
      }
      else if ( parameters["mode"].toString() == "vertical" )
      {
         processors.push_back( Filtering::Processor<Processor>( inpand_vertical_c, Constraint( CPU_NONE, 1, 1, 1, 1 ), 0 ) );
         processors.push_back( Filtering::Processor<Processor>( Inpand_vertical_inpand8_isse, Constraint(CPU_ISSE, 8, 1, 1, 1), 1));
         processors.push_back( Filtering::Processor<Processor>( Inpand_vertical_inpand8_3dnow, Constraint(CPU_3DNOW, 8, 1, 1, 1), 2));
         processors.push_back( Filtering::Processor<Processor>( Inpand_vertical_inpand8_sse2, Constraint(CPU_SSE2, 8, 1, 1, 1), 3));
         processors.push_back( Filtering::Processor<Processor>( Inpand_vertical_inpand8_asse2, Constraint(CPU_SSE2, 8, 1, 16, 16), 4));
     }
      else if ( parameters["mode"].toString() == "both" )
      {
         processors.push_back( Filtering::Processor<Processor>( inpand_both_c, Constraint( CPU_NONE, 1, 1, 1, 1 ), 0 ) );
         processors.push_back( Filtering::Processor<Processor>( Inpand_both_inpand8_isse, Constraint(CPU_ISSE, 8, 1, 1, 1), 1));
         processors.push_back( Filtering::Processor<Processor>( Inpand_both_inpand8_3dnow, Constraint(CPU_3DNOW, 8, 1, 1, 1), 2));
         processors.push_back( Filtering::Processor<Processor>( Inpand_both_inpand8_sse2, Constraint(CPU_SSE2, 8, 1, 1, 1), 3));
         processors.push_back( Filtering::Processor<Processor>( Inpand_both_inpand8_asse2, Constraint(CPU_SSE2, 8, 1, 16, 16), 4));
     }
      else
      {
         processors.push_back( Filtering::Processor<Processor>( inpand_custom_c, Constraint( CPU_NONE, 1, 1, 1, 1 ), 0 ) );
         FillCoordinates( parameters["mode"].toString() );
      }
   }

   static Signature Filter::filter_signature()
   {
      Signature signature = "mt_inpand";

      signature.add( Parameter( TYPE_CLIP, "" ) );
      signature.add( Parameter( 255, "thY" ) );
      signature.add( Parameter( 255, "thC" ) );
      signature.add( Parameter( String( "square" ), "mode" ) );

      return add_defaults( signature );
   }
};

} } } } } // namespace Inpand, Morphologic, Filter, MaskTools, Filtering

#endif