#include "inpand16.h"
#include "../functions16.h"

using namespace Filtering;

typedef Word (local_minimum_f)(Word a1, Word a2, Word a3, Word a4, Word a5, Word a6, Word a7, Word a8, Word a9);

static inline Word minimum_square(Word a1, Word a2, Word a3, Word a4, Word a5, Word a6, Word a7, Word a8, Word a9)
{
    Word nMin = a1;
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

static inline Word minimum_horizontal(Word a1, Word a2, Word a3, Word a4, Word a5, Word a6, Word a7, Word a8, Word a9)
{
    Word nMin = a4;

    UNUSED(a1); UNUSED(a2); UNUSED(a3); UNUSED(a7); UNUSED(a8); UNUSED(a9); 

    if ( a5 < nMin ) nMin = a5;
    if ( a6 < nMin ) nMin = a6;
    return nMin;
}

static inline Word minimum_vertical(Word a1, Word a2, Word a3, Word a4, Word a5, Word a6, Word a7, Word a8, Word a9)
{
    Word nMin = a2;

    UNUSED(a1); UNUSED(a3); UNUSED(a4); UNUSED(a6); UNUSED(a7); UNUSED(a9); 

    if ( a5 < nMin ) nMin = a5;
    if ( a8 < nMin ) nMin = a8;
    return nMin;
}

static inline Word minimum_both(Word a1, Word a2, Word a3, Word a4, Word a5, Word a6, Word a7, Word a8, Word a9)
{
    Word nMin = a2;

    UNUSED(a1); UNUSED(a3); UNUSED(a7); UNUSED(a9); 

    if ( a4 < nMin ) nMin = a4;
    if ( a5 < nMin ) nMin = a5;
    if ( a6 < nMin ) nMin = a6;
    if ( a8 < nMin ) nMin = a8;
    return nMin;
}

template<local_minimum_f Minimum>
static inline Word minimumThresholded(Word a1, Word a2, Word a3, Word a4, Word a5, Word a6, Word a7, Word a8, Word a9, int nMaxDeviation)
{
    int nMinimum = Minimum(a1, a2, a3, a4, a5, a6, a7, a8, a9);
    if ( a5 - nMinimum > nMaxDeviation ) nMinimum = a5 - nMaxDeviation;
    return static_cast<Word>(nMinimum);
}

namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic16 { namespace Inpand16 {

    class NewValue {
        int nMin;
        int nMaxDeviation;
        Word nValue;
    public:
        NewValue(Word nValue, int nMaxDeviation) : nMin(65536), nMaxDeviation(nMaxDeviation), nValue(nValue) { }
        void add(Word nValue) { if ( nValue < nMin ) nMin = nValue; }
        Word finalize() const { return static_cast<Word>(nMin > 65535 ? nValue : (nValue - nMin > nMaxDeviation ? nValue - nMaxDeviation : nMin)); }
    };

Processor *inpand_square_stacked_c = &generic_c<Byte,
    process_line_morpho_stacked_c<Border::Left, minimumThresholded<::minimum_square>>,
    process_line_morpho_stacked_c<Border::None, minimumThresholded<::minimum_square>>,
    process_line_morpho_stacked_c<Border::Right, minimumThresholded<::minimum_square>>
>;

Processor *inpand_horizontal_stacked_c = &generic_c<Byte,
    process_line_morpho_stacked_c<Border::Left, minimumThresholded<::minimum_horizontal>>,
    process_line_morpho_stacked_c<Border::None, minimumThresholded<::minimum_horizontal>>,
    process_line_morpho_stacked_c<Border::Right, minimumThresholded<::minimum_horizontal>>
    >;

Processor *inpand_vertical_stacked_c = &generic_c<Byte,
    process_line_morpho_stacked_c<Border::Left, minimumThresholded<::minimum_vertical>>,
    process_line_morpho_stacked_c<Border::None, minimumThresholded<::minimum_vertical>>,
    process_line_morpho_stacked_c<Border::Right, minimumThresholded<::minimum_vertical>>
    >;

Processor *inpand_both_stacked_c = &generic_c<Byte,
    process_line_morpho_stacked_c<Border::Left, minimumThresholded<::minimum_both>>,
    process_line_morpho_stacked_c<Border::None, minimumThresholded<::minimum_both>>,
    process_line_morpho_stacked_c<Border::Right, minimumThresholded<::minimum_both>>
    >;

Processor *inpand_custom_stacked_c       = &generic_custom_stacked_c<NewValue>;

} } } } }