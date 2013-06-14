#include "filter.h"

using namespace Filtering;


template<class Type>
class Left {
   const Byte *pSrc;
public:
   Left(const Byte *pSrc) : pSrc(pSrc) { }
   inline Type get(int nX, int nOffset) { if ( nX + nOffset < 0 ) return pSrc[0]; else return pSrc[nX + nOffset]; }
};

template<class Type>
class Right {
   const Byte *pSrc;
   int nWidth;
public:
   Right(const Byte *pSrc, int nWidth) : pSrc(pSrc), nWidth(nWidth) { }
   inline Type get(int nX, int nOffset) { if ( nX + nOffset >= nWidth ) return pSrc[nWidth-1]; else return pSrc[nX + nOffset]; }
};

template<class Type>
class Center {
   const Byte *pSrc;
public:
   Center(const Byte *pSrc) : pSrc(pSrc) { }
   inline Type get(int nX, int nOffset) { return pSrc[nX + nOffset]; }
};

template<class Type, class PixelFetcher>
Type compute_horizontal_vector_t(PixelFetcher &pf, const Type *horizontal, const int nHorizontal, int nOffset)
{
   Type nSum = 0;
   for ( int i = 0; i < nHorizontal; i++ )
      nSum += pf.get(i, nOffset) * horizontal[i];
   return nSum;
}

template<class Type>
void compute_line_t(const Byte *pSrc, const Type *horizontal, const int nHorizontal, int nWidth, Type *vectors)
{
   Left<Type> left(pSrc);
   for ( int i = -(nHorizontal / 2); i < 0; i++ )
      vectors[i + nHorizontal / 2] = compute_horizontal_vector_t<Type, Left<Type> >(left, horizontal, nHorizontal, i);

   Center<Type> center(pSrc);
   for ( int i = 0; i < nWidth - nHorizontal; i++ )
      vectors[i + nHorizontal / 2] = compute_horizontal_vector_t<Type, Center<Type> >(center, horizontal, nHorizontal, i);

   Right<Type> right(pSrc, nWidth);
   for ( int i = nWidth - nHorizontal; i < nWidth - (nHorizontal / 2); i++ )
      vectors[i + nHorizontal / 2] = compute_horizontal_vector_t<Type, Right<Type> >(right, horizontal, nHorizontal, i);
}

template<class Type>
Byte clamp(Type a);

template<> Byte clamp<int>(int a) { return a < 0 ? 0 : ( a > 255 ? 255 : Byte(a)); }
template<> Byte clamp<float>(float a) { return a < 0.0 ? 0 : ( a > 255.0 ? 255 : Byte(a+0.5) ); }

template<class Type>
struct NOP{
   static Type function(Type x) { return x; }
};

template<class Type>
struct MIRROR {
   static Type function(Type x) { return x < 0 ? -x : x; }
};

template<class Type, class SaturateOp>
class HorizontalVectors {
   Type **horizontals;
   Type **uncircled_horizontals;
   int nVertical;
   int nIdx;
public:
   HorizontalVectors() : horizontals(NULL), uncircled_horizontals(NULL) {}
   HorizontalVectors(int nWidth, int nVertical) : nVertical(nVertical), nIdx(0)
   {
      horizontals = new Type*[nVertical];
      for ( int i = 0; i < nVertical; i++ )
         horizontals[i] = new Type[nWidth];
      uncircled_horizontals = new Type*[nVertical];
   }
   ~HorizontalVectors()
   {
      if ( horizontals )
      {
         for ( int i = 0; i < nVertical; i++ )
            if ( horizontals[i] )
               delete[] horizontals[i];
         delete[] horizontals;
      }
   }
   Type *get_line(int nIdx)
   {
      if ( nIdx + this->nIdx < nVertical )
         return horizontals[nIdx + this->nIdx];
      else
         return horizontals[nIdx + this->nIdx - nVertical];
   }
   void next_line()
   {
      nIdx++;
      if ( nIdx >= nVertical )
         nIdx = 0;
   }
   void compute_verticals(Byte *pDst, const Type *vertical, int nWidth, Type nNormalization)
   {
      for ( int i = 0; i < nVertical; i++ )
         uncircled_horizontals[i] = get_line(i);

      for ( int i = 0; i < nWidth; i++ )
      {
         Type nSum = 0;
         for ( int j = 0; j < nVertical; j++ )
            nSum += uncircled_horizontals[j][i] * vertical[j];
         pDst[i] = clamp<Type>((SaturateOp::function(nSum) + nNormalization / 2) / nNormalization);
      }
   }
};

template<class Type> bool isNull(Type val) { UNUSED(val); return false; }
template<> bool isNull<double>(double val) { return val < 0.001f && val > -0.001f; }
template<> bool isNull<int>(int val) { return val == 0; }

template<class Type>
Type compute_norm_t(const Type *horizontal, const Type *vertical, int nHorizontal, int nVertical)
{
   Type nSum = 0;
   for ( int i = 0; i < nVertical; i++ )
      for ( int j = 0; j < nHorizontal; j++ )
         nSum += horizontal[j] * vertical[i];

   if ( isNull<Type>(nSum) )
      return 1;

   return nSum;
}


template<class Type, class SaturateOp>
void convolution_t(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, 
                   void *_horizontal, void *_vertical, void *_total, const int nHorizontal, const int nVertical,
                   int nWidth, int nHeight)
{
   const Type *horizontal = static_cast<const Type*>(_horizontal);
   const Type *vertical   = static_cast<const Type*>(_vertical);

   HorizontalVectors<Type, SaturateOp> *horizontals = NULL;
   const Type nNormalization = _total ? *static_cast<const Type*>(_total) : compute_norm_t<Type>(horizontal, vertical, nHorizontal, nVertical);

   if ( !horizontals )
      horizontals = new HorizontalVectors<Type, SaturateOp>(nWidth, nVertical);

   for ( int i = 0; i < nVertical / 2; i++ )
      compute_line_t<Type>(pSrc, horizontal, nHorizontal, nWidth, horizontals->get_line(i));

   for ( int i = nVertical / 2; i < nVertical - 1; i++ )
   {
      compute_line_t<Type>(pSrc, horizontal, nHorizontal, nWidth, horizontals->get_line(i));
      pSrc += nSrcPitch;
   }

   for ( int i = 0; i < nHeight - (nVertical / 2); i++ )
   {
      compute_line_t<Type>(pSrc, horizontal, nHorizontal, nWidth, horizontals->get_line(nVertical-1));
      horizontals->compute_verticals(pDst, vertical, nWidth, nNormalization);
      horizontals->next_line();
      pSrc += nSrcPitch;
      pDst += nDstPitch;
   }

   pSrc -= nSrcPitch;

   for ( int i = nHeight - (nVertical / 2); i < nHeight; i++ )
   {
      compute_line_t<Type>(pSrc, horizontal, nHorizontal, nWidth, horizontals->get_line(nVertical-1));
      horizontals->compute_verticals(pDst, vertical, nWidth, nNormalization);
      horizontals->next_line();
      pDst += nDstPitch;
   }

   delete horizontals;
}

namespace Filtering { namespace MaskTools { namespace Filters { namespace Convolution {

Processor *convolution_i_s_c = &::convolution_t<int, struct NOP<int> >;
Processor *convolution_f_s_c = &::convolution_t<float, struct NOP<float> >;
Processor *convolution_i_m_c = &::convolution_t<int, struct MIRROR<int> >;
Processor *convolution_f_m_c = &::convolution_t<float, struct MIRROR<float> >;

} } } }