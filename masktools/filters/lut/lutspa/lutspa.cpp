#include "lutspa.h"

using namespace Filtering;

void Filtering::MaskTools::Filters::Lut::Coordinate::lut_c(Byte *pDst, ptrdiff_t nDstPitch, int nWidth, int nHeight, const Byte lut[65536])
{
    Functions::copy_c(pDst, nDstPitch, lut, nWidth, nWidth, nHeight);
}
