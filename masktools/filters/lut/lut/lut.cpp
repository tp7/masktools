#include "lut.h"

using namespace Filtering;

void Filtering::MaskTools::Filters::Lut::Single::lut_c(Byte *dstp, ptrdiff_t dst_pitch, int width, int height, const Byte lut[256])
{
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++) {
            dstp[x] = lut[dstp[x]];
        }
        dstp += dst_pitch;
    }
}