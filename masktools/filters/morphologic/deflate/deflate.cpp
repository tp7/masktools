#include "deflate.h"
#include "../functions.h"
#include "../../../common/simd.h"

using namespace Filtering;

static inline Byte meanMin(Byte a1, Byte a2, Byte a3, Byte a4, Byte a5, Byte a6, Byte a7, Byte a8, Byte a9)
{
   int nSum = 0;
   nSum += a1 + a2 + a3 + a4 + a6 + a7 + a8 + a9;
   nSum >>= 3;
   return static_cast<Byte>(nSum < a5 ? nSum : a5);
}

static inline Byte meanMinThresholded(Byte a1, Byte a2, Byte a3, Byte a4, Byte a5, Byte a6, Byte a7, Byte a8, Byte a9, int nMaxDeviation)
{
   int nMeanMin = meanMin(a1, a2, a3, a4, a5, a6, a7, a8, a9);
   if ( a5 - nMeanMin > nMaxDeviation ) nMeanMin = a5 - nMaxDeviation;
   return static_cast<Byte>(nMeanMin);
}

namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic { namespace Deflate {

Processor *deflate_c = &generic_c<meanMinThresholded>;
Processor *deflate_sse2 = &generic_sse2<
    process_line_xxflate<Border::Left, limit_down_sse2, simd_loadu_epi128, simd_storeu_epi128>,
    process_line_xxflate<Border::None, limit_down_sse2, simd_loadu_epi128, simd_storeu_epi128>,
    process_line_xxflate<Border::Right, limit_down_sse2, simd_loadu_epi128, simd_storeu_epi128>
>;
Processor *deflate_asse2 = &generic_sse2<
    process_line_xxflate<Border::Left, limit_down_sse2, simd_load_epi128, simd_store_epi128>,
    process_line_xxflate<Border::Left, limit_down_sse2, simd_load_epi128, simd_store_epi128>,
    process_line_xxflate<Border::Left, limit_down_sse2, simd_load_epi128, simd_store_epi128>
>;

} } } } }