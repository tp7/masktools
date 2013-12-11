#include "lutf.h"
#include "../functions.h"

using namespace Filtering;

template<class T>
static void frame_c(Byte *dstp, ptrdiff_t dst_pitch, const Byte *srcp, ptrdiff_t src_pitch, const Byte *lutp, int width, int height)
{
    T processor("");

    processor.reset();

    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            processor.add(dstp[i]);
        }
        dstp += dst_pitch;
    }

    const Byte *lut = lutp + (processor.finalize() << 8);

    dstp -= dst_pitch * height;

    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            dstp[i] = lut[srcp[i]];
        }
        srcp += src_pitch;
        dstp += dst_pitch;
    }
}

namespace Filtering { namespace MaskTools { namespace Filters { namespace Lut { namespace Frame {

Processor *processors_array[NUM_MODES] = MPROCESSOR_SINGLE( frame_c );

} } } } }

