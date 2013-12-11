#ifndef __Common_BaseFunctions_H__
#define __Common_BaseFunctions_H__

#include "../utils/utils.h"
#include "../constraints/constraints.h"

namespace Filtering { namespace Functions {
 
void memset_plane(Byte *pPlane, ptrdiff_t nPitch, int nWidth, int nHeight, Byte value);
void copy_plane(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int Height);

CpuFlags get_cpu_flags();


}
}

#endif
