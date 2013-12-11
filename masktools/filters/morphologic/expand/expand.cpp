#include "expand.h"
#include "../functions.h"

using namespace Filtering;

typedef Byte (local_maximum_f)(Byte a1, Byte a2, Byte a3, Byte a4, Byte a5, Byte a6, Byte a7, Byte a8, Byte a9);

static inline Byte maximum_square(Byte a1, Byte a2, Byte a3, Byte a4, Byte a5, Byte a6, Byte a7, Byte a8, Byte a9)
{
    Byte nMax = a1;
    if (a2 > nMax) nMax = a2;
    if (a3 > nMax) nMax = a3;
    if (a4 > nMax) nMax = a4;
    if (a5 > nMax) nMax = a5;
    if (a6 > nMax) nMax = a6;
    if (a7 > nMax) nMax = a7;
    if (a8 > nMax) nMax = a8;
    if (a9 > nMax) nMax = a9;
    return nMax;
}

static inline Byte maximum_horizontal(Byte a1, Byte a2, Byte a3, Byte a4, Byte a5, Byte a6, Byte a7, Byte a8, Byte a9)
{
    Byte nMax = a4;

    UNUSED(a1); UNUSED(a2); UNUSED(a3); UNUSED(a7); UNUSED(a8); UNUSED(a9);

    if (a5 > nMax) nMax = a5;
    if (a6 > nMax) nMax = a6;
    return nMax;
}

static inline Byte maximum_vertical(Byte a1, Byte a2, Byte a3, Byte a4, Byte a5, Byte a6, Byte a7, Byte a8, Byte a9)
{
    Byte nMax = a2;

    UNUSED(a1); UNUSED(a3); UNUSED(a4); UNUSED(a6); UNUSED(a7); UNUSED(a9);

    if (a5 > nMax) nMax = a5;
    if (a8 > nMax) nMax = a8;
    return nMax;
}

static inline Byte maximum_both(Byte a1, Byte a2, Byte a3, Byte a4, Byte a5, Byte a6, Byte a7, Byte a8, Byte a9)
{
    Byte nMax = a2;

    UNUSED(a1); UNUSED(a3); UNUSED(a7); UNUSED(a9);

    if (a4 > nMax) nMax = a4;
    if (a5 > nMax) nMax = a5;
    if (a6 > nMax) nMax = a6;
    if (a8 > nMax) nMax = a8;
    return nMax;
}

template<local_maximum_f Maximum>
static inline Byte maximumThresholded(Byte a1, Byte a2, Byte a3, Byte a4, Byte a5, Byte a6, Byte a7, Byte a8, Byte a9, int nMaxDeviation)
{
    int nMaximum = Maximum(a1, a2, a3, a4, a5, a6, a7, a8, a9);
    if (nMaximum - a5 > nMaxDeviation) {
        nMaximum = a5 + nMaxDeviation;
    }
    return static_cast<Byte>(nMaximum);
}

extern "C" static MT_FORCEINLINE __m128i expand_operator_sse2(__m128i a, __m128i b) {
    return _mm_max_epu8(a, b);
}

namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic { namespace Expand {

class ExpandProcessor {
    int max;
    int max_deviation;
    Byte value;
public:
    ExpandProcessor(Byte nValue, int nMaxDeviation) : max(-1), max_deviation(nMaxDeviation), value(nValue) { }

    void add(Byte nValue) {
        if (nValue > max) {
            max = nValue;
        }
    }

    Byte finalize() const {
        return static_cast<Byte>(max < 0 ? value : (max - value > max_deviation ? value + max_deviation : max));
    }
};

Processor *expand_square_c       = &generic_c<maximumThresholded<::maximum_square> >;
Processor *expand_both_c         = &generic_c<maximumThresholded<::maximum_both> >;
Processor *expand_horizontal_c   = &generic_c<maximumThresholded<::maximum_horizontal> >;
Processor *expand_vertical_c     = &generic_c<maximumThresholded<::maximum_vertical> >;


#define DEFINE_SSE2_VERSIONS(name, mem_mode) \
    Processor *expand_both_##name = &generic_sse2< \
    process_line_xxpand_both<Border::Left, expand_operator_sse2, limit_up_sse2, mem_mode>, \
    process_line_xxpand_both<Border::None, expand_operator_sse2, limit_up_sse2, mem_mode>, \
    process_line_xxpand_both<Border::Right, expand_operator_sse2, limit_up_sse2, MemoryMode::SSE2_UNALIGNED> \
    >;

DEFINE_SSE2_VERSIONS(sse2, MemoryMode::SSE2_UNALIGNED)
DEFINE_SSE2_VERSIONS(asse2, MemoryMode::SSE2_ALIGNED)

Processor *expand_vertical_sse2 = &xxpand_sse2_vertical<expand_operator_sse2, limit_up_sse2, MemoryMode::SSE2_UNALIGNED>;
Processor *expand_vertical_asse2 = &xxpand_sse2_vertical<expand_operator_sse2, limit_up_sse2, MemoryMode::SSE2_ALIGNED>;

Processor *expand_horizontal_sse2 = &xxpand_sse2_horizontal<expand_operator_sse2, limit_up_sse2, MemoryMode::SSE2_UNALIGNED, expand_c_horizontal_core>;
Processor *expand_horizontal_asse2 = &xxpand_sse2_horizontal<expand_operator_sse2, limit_up_sse2, MemoryMode::SSE2_ALIGNED, expand_c_horizontal_core>;

Processor *expand_square_sse2 = &xxpand_sse2_square<expand_operator_sse2, limit_up_sse2, MemoryMode::SSE2_UNALIGNED, expand_c_horizontal_core>;
Processor *expand_square_asse2 = &xxpand_sse2_square<expand_operator_sse2, limit_up_sse2, MemoryMode::SSE2_ALIGNED, expand_c_horizontal_core>;

Processor *expand_custom_c = &generic_custom_c<ExpandProcessor>;

} } } } }