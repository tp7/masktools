#include "filter.h"
#include "../../../filters/morphologic/functions.h"

using namespace Filtering;

static inline Byte meanMax(Byte a1, Byte a2, Byte a3, Byte a4, Byte a5, Byte a6, Byte a7, Byte a8, Byte a9)
{
   int nSum = 0;
   nSum += a1 + a2 + a3 + a4 + a6 + a7 + a8 + a9;
   nSum >>= 3;
   return static_cast<Byte>(nSum > a5 ? nSum : a5);
}

static inline Byte meanMaxThresholded(Byte a1, Byte a2, Byte a3, Byte a4, Byte a5, Byte a6, Byte a7, Byte a8, Byte a9, int nMaxDeviation)
{
   int nMeanMax = meanMax(a1, a2, a3, a4, a5, a6, a7, a8, a9);
   if ( nMeanMax - a5 > nMaxDeviation ) nMeanMax = a5 + nMaxDeviation;
   return static_cast<Byte>(nMeanMax);
}

namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic { namespace Inflate {

Processor *inflate_c = &generic_c<meanMaxThresholded>;

} } } } }