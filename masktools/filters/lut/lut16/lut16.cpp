#include "lut16.h"

using namespace Filtering;

namespace Filtering { namespace MaskTools { namespace Filters { namespace Lut { namespace Single16bit { 


void lut16_c_interleaved(Byte *pDst, ptrdiff_t nDstPitch, int nWidth, int nHeight, const Word lut[65536])
{
    for ( int y = 0; y < nHeight; y++ )
    {
        auto pDst16 = reinterpret_cast<Word*>(pDst);
        for ( int x = 0; x < nWidth / 2; x+=1 )
            pDst16[x] = lut[pDst16[x]];
        pDst += nDstPitch;
    }
}

void lut16_c_stacked(Byte *pDst, ptrdiff_t nDstPitch, int nWidth, int nHeight, const Word lut[65536])
{
    auto pLsb = pDst + nDstPitch * nHeight / 2;

    for ( int y = 0; y < nHeight / 2; y++ )
    {
        for ( int x = 0; x < nWidth; x++ ) {
            Word value = lut[(pDst[x] << 8) + pLsb[x]];
            pDst[x] = value >> 8;
            pLsb[x] = value & 0xFF;
        }
        pDst += nDstPitch;
        pLsb += nDstPitch;
    }
}

} } } } }