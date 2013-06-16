#include "lutspa.h"

using namespace Filtering;

void Filtering::MaskTools::Filters::Lut::Coordinate::lut_c(Byte *pDst, ptrdiff_t nDstPitch, int nWidth, int nHeight, const Byte lut[65536])
{
    if (nDstPitch == nWidth) {
        memcpy(pDst, lut, nWidth*nHeight);
    } else {
        for (int y = 0; y < nHeight; ++y) {
            memcpy(pDst, lut, nWidth);
            pDst += nDstPitch;
            lut += nWidth;
        }
    }
}

