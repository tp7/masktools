#include "filter.h"
#include "../../../filters/morphologic/functions.h"

using namespace Filtering;

typedef Byte (local_minimum_f)(Byte a1, Byte a2, Byte a3, Byte a4, Byte a5, Byte a6, Byte a7, Byte a8, Byte a9);

static inline Byte minimum_square(Byte a1, Byte a2, Byte a3, Byte a4, Byte a5, Byte a6, Byte a7, Byte a8, Byte a9)
{
   Byte nMin = a1;
   if ( a2 < nMin ) nMin = a2;
   if ( a3 < nMin ) nMin = a3;
   if ( a4 < nMin ) nMin = a4;
   if ( a5 < nMin ) nMin = a5;
   if ( a6 < nMin ) nMin = a6;
   if ( a7 < nMin ) nMin = a7;
   if ( a8 < nMin ) nMin = a8;
   if ( a9 < nMin ) nMin = a9;
   return nMin;
}

static inline Byte minimum_horizontal(Byte a1, Byte a2, Byte a3, Byte a4, Byte a5, Byte a6, Byte a7, Byte a8, Byte a9)
{
   Byte nMin = a4;

   UNUSED(a1); UNUSED(a2); UNUSED(a3); UNUSED(a7); UNUSED(a8); UNUSED(a9); 

   if ( a5 < nMin ) nMin = a5;
   if ( a6 < nMin ) nMin = a6;
   return nMin;
}

static inline Byte minimum_vertical(Byte a1, Byte a2, Byte a3, Byte a4, Byte a5, Byte a6, Byte a7, Byte a8, Byte a9)
{
   Byte nMin = a2;

   UNUSED(a1); UNUSED(a3); UNUSED(a4); UNUSED(a6); UNUSED(a7); UNUSED(a9); 

   if ( a5 < nMin ) nMin = a5;
   if ( a8 < nMin ) nMin = a8;
   return nMin;
}

static inline Byte minimum_both(Byte a1, Byte a2, Byte a3, Byte a4, Byte a5, Byte a6, Byte a7, Byte a8, Byte a9)
{
   Byte nMin = a2;

   UNUSED(a1); UNUSED(a3); UNUSED(a7); UNUSED(a9); 

   if ( a4 < nMin ) nMin = a4;
   if ( a5 < nMin ) nMin = a5;
   if ( a6 < nMin ) nMin = a6;
   if ( a8 < nMin ) nMin = a8;
   return nMin;
}

template<local_minimum_f Minimum>
static inline Byte minimumThresholded(Byte a1, Byte a2, Byte a3, Byte a4, Byte a5, Byte a6, Byte a7, Byte a8, Byte a9, int nMaxDeviation)
{
   int nMinimum = Minimum(a1, a2, a3, a4, a5, a6, a7, a8, a9);
   if ( a5 - nMinimum > nMaxDeviation ) nMinimum = a5 - nMaxDeviation;
   return static_cast<Byte>(nMinimum);
}

namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic { namespace Inpand {

class NewValue {
   int nMin;
   int nMaxDeviation;
   Byte nValue;
public:
   NewValue(Byte nValue, int nMaxDeviation) : nMin(256), nValue(nValue), nMaxDeviation(nMaxDeviation) { }
   void add(Byte nValue) { if ( nValue < nMin ) nMin = nValue; }
   Byte finalize() const { return static_cast<Byte>(nMin > 255 ? nValue : (nValue - nMin > nMaxDeviation ? nValue - nMaxDeviation : nMin)); }
};

Processor *inpand_square_c       = &generic_c<minimumThresholded<::minimum_square> >;
Processor *inpand_horizontal_c   = &generic_c<minimumThresholded<::minimum_horizontal> >;
Processor *inpand_vertical_c     = &generic_c<minimumThresholded<::minimum_vertical> >;
Processor *inpand_both_c         = &generic_c<minimumThresholded<::minimum_both> >;

Processor *inpand_custom_c       = &generic_custom_c<NewValue>;

} } } } }