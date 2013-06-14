#include "filter.h"
#include "../../../filters/mask/functions.h"

using namespace Filtering;

namespace Filtering { namespace MaskTools { namespace Filters { namespace Mask { namespace Edge {

static inline Byte convolution(Byte a11, Byte a21, Byte a31, Byte a12, Byte a22, Byte a32, Byte a13, Byte a23, Byte a33, const Short matrix[10], int nLowThreshold, int nHighThreshold)
{
   return threshold<Byte, int>(abs((a11 * matrix[0] + a21 * matrix[1] + a31 * matrix[2] + 
                                    a12 * matrix[3] + a22 * matrix[4] + a32 * matrix[5] +
                                    a13 * matrix[6] + a23 * matrix[7] + a33 * matrix[8]) / matrix[9]), nLowThreshold, nHighThreshold);
}

static inline Byte sobel(Byte a11, Byte a21, Byte a31, Byte a12, Byte a22, Byte a32, Byte a13, Byte a23, Byte a33, const Short matrix[10], int nLowThreshold, int nHighThreshold)
{
   UNUSED(a11); UNUSED(a13); UNUSED(a22); UNUSED(a31); UNUSED(a33); UNUSED(matrix); 
   return threshold<Byte, int>(abs( (int)a32 + a23 - a12 - a21 ) >> 1, nLowThreshold, nHighThreshold);
}

static inline Byte roberts(Byte a11, Byte a21, Byte a31, Byte a12, Byte a22, Byte a32, Byte a13, Byte a23, Byte a33, const Short matrix[10], int nLowThreshold, int nHighThreshold)
{
   UNUSED(a11); UNUSED(a12); UNUSED(a13); UNUSED(a21); UNUSED(a31); UNUSED(a33); UNUSED(matrix); 
   return threshold<Byte, int>(abs( ((int)a22 << 1) - a32 - a23 ) >> 1, nLowThreshold, nHighThreshold);
}

static inline Byte laplace(Byte a11, Byte a21, Byte a31, Byte a12, Byte a22, Byte a32, Byte a13, Byte a23, Byte a33, const Short matrix[10], int nLowThreshold, int nHighThreshold)
{
   UNUSED(matrix); 
   return threshold<Byte, int>(abs( ((int)a22 << 3) - a32 - a23 - a11 - a21 - a31 - a12 - a13 - a33 ) >> 3, nLowThreshold, nHighThreshold);
}

static inline Byte morpho(Byte a11, Byte a21, Byte a31, Byte a12, Byte a22, Byte a32, Byte a13, Byte a23, Byte a33, const Short matrix[10], int nLowThreshold, int nHighThreshold)
{
   int nMin = a11, nMax = a11;

   UNUSED(matrix); 

   nMin = min<int>( nMin, a21 );
   nMax = max<int>( nMax, a21 );
   nMin = min<int>( nMin, a31 );
   nMax = max<int>( nMax, a31 );
   nMin = min<int>( nMin, a12 );
   nMax = max<int>( nMax, a12 );
   nMin = min<int>( nMin, a22 );
   nMax = max<int>( nMax, a22 );
   nMin = min<int>( nMin, a32 );
   nMax = max<int>( nMax, a32 );
   nMin = min<int>( nMin, a13 );
   nMax = max<int>( nMax, a13 );
   nMin = min<int>( nMin, a23 );
   nMax = max<int>( nMax, a23 );
   nMin = min<int>( nMin, a33 );
   nMax = max<int>( nMax, a33 );

   return threshold<Byte, int>( nMax - nMin, nLowThreshold, nHighThreshold );
}

static inline Byte cartoon(Byte a11, Byte a21, Byte a31, Byte a12, Byte a22, Byte a32, Byte a13, Byte a23, Byte a33, const Short matrix[10], int nLowThreshold, int nHighThreshold)
{
   int val = ((int)a21 << 1) - a22 - a31;

   UNUSED(a11); UNUSED(a12); UNUSED(a13); UNUSED(a23); UNUSED(a32); UNUSED(a33); UNUSED(matrix); 

   return val > 0 ? 0 : threshold<Byte, int>( -val, nLowThreshold, nHighThreshold );
}

static inline Byte prewitt(Byte a11, Byte a21, Byte a31, Byte a12, Byte a22, Byte a32, Byte a13, Byte a23, Byte a33, const Short matrix[10], int nLowThreshold, int nHighThreshold)
{
   const int p90 = a11 + a21 + a31 - a13 - a23 - a33;
   const int p180 = a11 + a12 + a13 - a31 - a32 - a33;
   const int p45 = a12 + a11 + a21 - a33 - a32 - a23;
   const int p135 = a13 + a12 + a23 - a31 - a32 - a21;

   const int max1 = max<int>( abs<int>( p90 ), abs<int>( p180 ) );
   const int max2 = max<int>( abs<int>( p45 ), abs<int>( p135 ) );
   const int maxv = max<int>( max1, max2 );

   UNUSED(a22); UNUSED(matrix); 

   return threshold<Byte, int>( maxv, nLowThreshold, nHighThreshold );
}

static inline Byte half_prewitt(Byte a11, Byte a21, Byte a31, Byte a12, Byte a22, Byte a32, Byte a13, Byte a23, Byte a33, const Short matrix[10], int nLowThreshold, int nHighThreshold)
{
   const int p90 = a11 + 2 * a21 + a31 - a13 - 2 * a23 - a33;
   const int p180 = a11 + 2 * a12 + a13 - a31 - 2 * a32 - a33;
   const int maxv = max<int>( abs<int>( p90 ), abs<int>( p180 ) );

   UNUSED(a22); UNUSED(matrix);
   
   return threshold<Byte, int>( maxv, nLowThreshold, nHighThreshold );
}

class Thresholds {
   Byte nMinThreshold, nMaxThreshold;
public:
   Thresholds(Byte nMinThreshold, Byte nMaxThreshold) :
   nMinThreshold(nMinThreshold), nMaxThreshold(nMaxThreshold)
   {
   }

   int minpitch() const { return 0; }
   int maxpitch() const { return 0; }
   void nextRow() { }
   Byte min(int x) const { UNUSED(x); return nMinThreshold; }
   Byte max(int x) const { UNUSED(x); return nMaxThreshold; }
};

template<Filters::Mask::Operator op>
void mask_t(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, const Short matrix[10], int nLowThreshold, int nHighThreshold, int nWidth, int nHeight)
{
   Thresholds thresholds(static_cast<Byte>(nLowThreshold), static_cast<Byte>(nHighThreshold));

   Filters::Mask::generic_c<op, Thresholds>(pDst, nDstPitch, pSrc, nSrcPitch, thresholds, matrix, nWidth, nHeight);
}

extern "C" Processor Edge_sobel8_mmx;
extern "C" Processor Edge_sobel8_sse2;
extern "C" Processor Edge_roberts8_mmx;
extern "C" Processor Edge_roberts8_sse2;
extern "C" Processor Edge_laplace8_mmx;
extern "C" Processor Edge_laplace8_sse2;
extern "C" Processor Edge_morpho8_isse;
extern "C" Processor Edge_morpho8_sse2;
extern "C" Processor Edge_convolution8_mmx;
extern "C" Processor Edge_convolution8_sse2;
extern "C" Processor Edge_prewitt8_isse;
extern "C" Processor Edge_prewitt8_sse2;
extern "C" Processor Edge_prewitt8_ssse3;
extern "C" Processor Edge_half_prewitt8_isse;
extern "C" Processor Edge_half_prewitt8_sse2;
extern "C" Processor Edge_half_prewitt8_ssse3;

Processor *convolution_c = &mask_t<convolution>;
Processor *convolution8_mmx = &Edge_convolution8_mmx;
Processor *convolution8_sse2 = &Edge_convolution8_sse2;

Processor *sobel_c = &mask_t<sobel>;
Processor *sobel8_mmx = &Edge_sobel8_mmx;
Processor *sobel8_sse2 = &Edge_sobel8_sse2;

Processor *roberts_c = &mask_t<roberts>;
Processor *roberts8_mmx = &Edge_roberts8_mmx;
Processor *roberts8_sse2 = &Edge_roberts8_sse2;

Processor *laplace_c = &mask_t<laplace>;
Processor *laplace8_mmx = &Edge_laplace8_mmx;
Processor *laplace8_sse2 = &Edge_laplace8_sse2;

Processor *prewitt_c = &mask_t<prewitt>;
Processor *prewitt8_isse = &Edge_prewitt8_isse;
Processor *prewitt8_sse2 = &Edge_prewitt8_sse2;
Processor *prewitt8_ssse3 = &Edge_prewitt8_ssse3;

Processor *half_prewitt_c = &mask_t<half_prewitt>;
Processor *half_prewitt8_isse = &Edge_half_prewitt8_isse;
Processor *half_prewitt8_sse2 = &Edge_half_prewitt8_sse2;
Processor *half_prewitt8_ssse3 = &Edge_half_prewitt8_ssse3;

Processor *cartoon_c = &mask_t<cartoon>;

Processor *morpho_c = &mask_t<morpho>;
Processor *morpho8_isse = &Edge_morpho8_isse;
Processor *morpho8_sse2 = &Edge_morpho8_sse2;

} } } } }