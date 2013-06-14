#ifndef __Mt_HelpersForms_H__
#define __Mt_HelpersForms_H__

#include "../../../common/utils/utils.h"

namespace Filtering { namespace MaskTools { namespace Helpers { namespace Forms {

typedef String (Form)(int nTopLeftX, int nTopLeftY, int nBottomRightX, int nBottomRightY, bool zero);

Form RectangleToString;
Form LosangeToString;
Form EllipseToString;

} } } }

#endif