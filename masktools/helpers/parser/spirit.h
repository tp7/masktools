#ifndef __Mt_SpiritParser_H__
#define __Mt_SpiritParser_H__

#include "../../common/common.h"

namespace Filtering { namespace MaskTools { namespace Helpers { namespace PolishConverter {

typedef String(StringConverter)(const String &);

StringConverter Converter;
StringConverter Infix;

} } } }

#endif