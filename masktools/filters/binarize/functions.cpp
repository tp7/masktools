#include "filter.h"

using namespace Filtering;

typedef Byte (Operator)(Byte, Byte);

static inline Byte upper(Byte x, Byte t) { return x > t ? 0 : 255; }
static inline Byte lower(Byte x, Byte t) { return x > t ? 255 : 0; }

static inline Byte binarize_0_x(Byte x, Byte t) { return x > t ? 0 : x; }
static inline Byte binarize_t_x(Byte x, Byte t) { return x > t ? t : x; }
static inline Byte binarize_x_0(Byte x, Byte t) { return x > t ? x : 0; }
static inline Byte binarize_x_t(Byte x, Byte t) { return x > t ? x : t; }

static inline Byte binarize_t_0(Byte x, Byte t) { return x > t ? t : 0; }
static inline Byte binarize_0_t(Byte x, Byte t) { return x > t ? 0 : t; }

static inline Byte binarize_x_255(Byte x, Byte t) { return x > t ? x : 255; }
static inline Byte binarize_t_255(Byte x, Byte t) { return x > t ? t : 255; }
static inline Byte binarize_255_x(Byte x, Byte t) { return x > t ? 255 : x; }
static inline Byte binarize_255_t(Byte x, Byte t) { return x > t ? 255 : t; }


template <Operator op>
void binarize_t(Byte *pDst, ptrdiff_t nDstPitch, Byte nThreshold, int nWidth, int nHeight)
{
   for ( int j = 0; j < nHeight; j++, pDst += nDstPitch )
      for ( int i = 0; i < nWidth; i++ )
         pDst[i] = op(pDst[i], nThreshold);
}

namespace Filtering { namespace MaskTools { namespace Filters { namespace Binarize {

Processor *upper_c = &binarize_t<upper>;
Processor *lower_c = &binarize_t<lower>;
Processor *binarize_0_x_c = &binarize_t<binarize_0_x>;
Processor *binarize_t_x_c = &binarize_t<binarize_t_x>;
Processor *binarize_x_0_c = &binarize_t<binarize_x_0>;
Processor *binarize_x_t_c = &binarize_t<binarize_x_t>;
Processor *binarize_t_0_c = &binarize_t<binarize_t_0>;
Processor *binarize_0_t_c = &binarize_t<binarize_0_t>;
Processor *binarize_x_255_c = &binarize_t<binarize_x_255>;
Processor *binarize_t_255_c = &binarize_t<binarize_t_255>;
Processor *binarize_255_x_c = &binarize_t<binarize_255_x>;
Processor *binarize_255_t_c = &binarize_t<binarize_255_t>;

} } } }