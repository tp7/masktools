#include "inpand.h"
#include "../functions.h"

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

extern "C" static MT_FORCEINLINE __m128i inpand_operator_sse2(__m128i a, __m128i b) {
    return _mm_min_epu8(a, b);
}

namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic { namespace Inpand {

class NewValue {
   int nMin;
   int nMaxDeviation;
   Byte nValue;
public:
   NewValue(Byte nValue, int nMaxDeviation) : nMin(256), nMaxDeviation(nMaxDeviation), nValue(nValue) { }
   void add(Byte nValue) { if ( nValue < nMin ) nMin = nValue; }
   Byte finalize() const { return static_cast<Byte>(nMin > 255 ? nValue : (nValue - nMin > nMaxDeviation ? nValue - nMaxDeviation : nMin)); }
};

Processor *inpand_square_c       = &generic_c<minimumThresholded<::minimum_square> >;
Processor *inpand_horizontal_c   = &generic_c<minimumThresholded<::minimum_horizontal> >;
Processor *inpand_vertical_c     = &generic_c<minimumThresholded<::minimum_vertical> >;
Processor *inpand_both_c         = &generic_c<minimumThresholded<::minimum_both> >;

#define DEFINE_SSE2_VERSIONS(name, mem_mode) \
    Processor *inpand_both_##name = &generic_sse2< \
    process_line_xxpand_both<Border::Left, inpand_operator_sse2, limit_down_sse2, mem_mode>, \
    process_line_xxpand_both<Border::None, inpand_operator_sse2, limit_down_sse2, mem_mode>, \
    process_line_xxpand_both<Border::Right, inpand_operator_sse2, limit_down_sse2, MemoryMode::SSE2_UNALIGNED> \
    >;

DEFINE_SSE2_VERSIONS(sse2, MemoryMode::SSE2_UNALIGNED)
DEFINE_SSE2_VERSIONS(asse2, MemoryMode::SSE2_ALIGNED)

Processor *inpand_vertical_sse2 = &xxpand_sse2_vertical<inpand_operator_sse2, limit_down_sse2, MemoryMode::SSE2_UNALIGNED>;
Processor *inpand_vertical_asse2 = &xxpand_sse2_vertical<inpand_operator_sse2, limit_down_sse2, MemoryMode::SSE2_ALIGNED>;

Processor *inpand_horizontal_sse2 = &xxpand_sse2_horizontal<inpand_operator_sse2, limit_down_sse2, MemoryMode::SSE2_UNALIGNED, inpand_c_horizontal_core>;
Processor *inpand_horizontal_asse2 = &xxpand_sse2_horizontal<inpand_operator_sse2, limit_down_sse2, MemoryMode::SSE2_ALIGNED, inpand_c_horizontal_core>;

Processor *inpand_square_sse2 = &xxpand_sse2_square<inpand_operator_sse2, limit_down_sse2, MemoryMode::SSE2_UNALIGNED, inpand_c_horizontal_core>;
Processor *inpand_square_asse2 = &xxpand_sse2_square<inpand_operator_sse2, limit_down_sse2, MemoryMode::SSE2_ALIGNED, inpand_c_horizontal_core>;


Processor *inpand_custom_c = &generic_custom_c<NewValue>;

} } } } }