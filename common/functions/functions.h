#ifndef __Common_BaseFunctions_H__
#define __Common_BaseFunctions_H__

#include "../utils/utils.h"
#include "../constraints/constraints.h"

namespace Filtering { namespace Functions {
 
void memset_plane(Byte *ptr, ptrdiff_t pitch, int width, int height, Byte value);
void copy_plane(Byte *pDst, ptrdiff_t dst_pitch, const Byte *pSrc, ptrdiff_t src_pitch, int width, int height);

CpuFlags get_cpu_flags();


}
}

#endif
