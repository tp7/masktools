#ifndef __Common_BaseFunctions_H__
#define __Common_BaseFunctions_H__

#include "../utils/utils.h"
#include "../constraints/constraints.h"

namespace Filtering { namespace Functions {
 
typedef void (Memset)(Byte *pPlane, ptrdiff_t nPitch, int nWidth, int nHeight, Byte value);

typedef void (Copy)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch,
                    int nWidth, int Height);

Memset memset_c;
Copy copy_c;

CpuFlags get_cpu_flags();

class Asm {
    Byte *pbBytes;
public:
    Asm();
    ~Asm();
};

}

// don't ask
extern "C" Functions::Memset memset8_mmx;
extern "C" Functions::Memset memset8_isse;
extern "C" Functions::Memset memset8_3dnow;

extern "C" Functions::Copy copy8_mmx;
extern "C" Functions::Copy copy8_isse;
extern "C" Functions::Copy copy8_3dnow;

}

#endif
