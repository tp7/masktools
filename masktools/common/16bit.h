#ifndef __Mt_16bit_H__
#define __Mt_16bit_H__

#include "common.h"

namespace Filtering {

MT_FORCEINLINE Word read_word_stacked(const Byte *pMsb, const Byte *pLsb, int x) {
    return (Word(pMsb[x]) << 8) + pLsb[x];
}

MT_FORCEINLINE void write_word_stacked(Byte *pMsb, Byte *pLsb, int x, Word value) {
    pMsb[x] = value >> 8;
    pLsb[x] = value & 0xFF;
}

}

#endif __Mt_16bit_H__
