#ifndef __Mt_COMMON_H__
#define __Mt_COMMON_H__

#include "../../common/utils/utils.h"

//because ICC is smart enough on its own and force inlining actually makes it slower
#ifdef __INTEL_COMPILER
#define MT_FORCEINLINE inline
#else
#define MT_FORCEINLINE __forceinline
#endif

#endif __Mt_COMMON_H__
