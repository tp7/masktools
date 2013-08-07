#include "inflate16.h"
#include "../functions16.h"
#include "../../../common/simd.h"

using namespace Filtering;

static inline Word meanMax(Word a1, Word a2, Word a3, Word a4, Word a5, Word a6, Word a7, Word a8, Word a9)
{
    int nSum = 0;
    nSum += a1 + a2 + a3 + a4 + a6 + a7 + a8 + a9;
    nSum >>= 3;
    return static_cast<Word>(nSum > a5 ? nSum : a5);
}

static inline Word meanMaxThresholded(Word a1, Word a2, Word a3, Word a4, Word a5, Word a6, Word a7, Word a8, Word a9, int nMaxDeviation)
{
    int nMeanMax = meanMax(a1, a2, a3, a4, a5, a6, a7, a8, a9);
    if ( nMeanMax - a5 > nMaxDeviation ) nMeanMax = a5 + nMaxDeviation;
    return static_cast<Word>(nMeanMax);
}

namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic16 { namespace Inflate16 {

StackedProcessor *inflate_stacked_c = &MorphologicProcessor<Byte>::generic_c<
    process_line_morpho_stacked_c<Border::Left, meanMaxThresholded>,
    process_line_morpho_stacked_c<Border::None, meanMaxThresholded>,
    process_line_morpho_stacked_c<Border::Right, meanMaxThresholded>
>;

InterleavedProcessor *inflate_interleaved_c = &MorphologicProcessor<Word>::generic_c<
    process_line_morpho_interleaved_c<Border::Left, meanMaxThresholded>,
    process_line_morpho_interleaved_c<Border::None, meanMaxThresholded>,
    process_line_morpho_interleaved_c<Border::Right, meanMaxThresholded>
>;

} } } } }