#ifndef __Mt_Lut_Helpers_H__
#define __Mt_Lut_Helpers_H__

namespace Filtering { namespace MaskTools { namespace Filters { namespace Lut {
    
static inline bool stringValueEmpty(const Filtering::Value &value) {
    return !value.is_defined() || value.toString().empty();
}

} } } }

#endif