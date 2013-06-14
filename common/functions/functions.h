#ifndef __Common_BaseFunctions_H__
#define __Common_BaseFunctions_H__

#include "../utils/utils.h"
#include "../constraints/constraints.h"

namespace Filtering { namespace Functions {
 
typedef void (Memset)(Byte *pPlane, ptrdiff_t nPitch, int nWidth, int nHeight, Byte value);

typedef void (Copy)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch,
                    int nWidth, int Height);

typedef void (Prefetch)(const Byte *pDst, int nWidth);

typedef void (Emms)();

Memset memset_c;
Copy copy_c;
Prefetch prefetch_nothing_c;

CpuFlags get_cpu_flags();

class Asm {
    Byte *pbBytes;
public:
    Asm();
    ~Asm();
};

class Dct {
protected:
   Double *pMatrix;
   int size;
public:
    Dct(int size) : size(size)
   {
      pMatrix = new Double[size * size];

      for ( int k = 0; k < size; k++ )
         for ( int n = 0; n < size; n++ )
            pMatrix[ k * size + n ] = ((k ? sqrt( 2.0 ) : 1.0) / sqrt( size * 1.0f )) * cos( (k * (2 * n + 1) * M_PI ) / (2 * size) );
   }
   ~Dct()
   {
      delete[] pMatrix;
   }
   void fdct(Double *pCoefficients, int nCoefficientsStride, const Double *pSource, int nSourceStride) const
   {
      for ( int k = 0; k < size; k++ )
      {
         Double value = 0;
         for ( int n = 0; n < size; n++ )
            value += pMatrix[ k * size + n ] * pSource[ n * nSourceStride ];
         pCoefficients[ k * nCoefficientsStride ] = value;
      }
   }
   void idct(Double *pCoefficients, int nCoefficientsStride, const Double *pSource, int nSourceStride) const
   {
      for ( int k = 0; k < size; k++ )
      {
         Double value = 0;
         for ( int n = 0; n < size; n++ )
            value += pMatrix[ n * size + k ] * pSource[ n * nSourceStride ];
         pCoefficients[ k * nCoefficientsStride ] = value;
      }
   }
   int get_size() const { return size; }
};

class Dct2D {
protected:
   Dct x, y;
   Double *pWorking;
public:
   Dct2D(int size_x, int size_y) : x(size_x), y(size_y)
   {
      pWorking = new Double[size_x * size_y];
   }
   ~Dct2D()
   {
      delete[] pWorking;
   }
   void fdct(Double *pCoefficients, int nCoefficientsPitch, const Double *pSource, int nSourcePitch) const
   {
      for ( int i = 0; i < y.get_size(); i++ )
         x.fdct( pWorking + i * x.get_size(), 1, pSource + i * nSourcePitch, 1 );
      for ( int i = 0; i < x.get_size(); i++ )
         y.fdct( pCoefficients + i, nCoefficientsPitch, pWorking + i, x.get_size() );
   }
   void idct(Double *pCoefficients, int nCoefficientsPitch, const Double *pSource, int nSourcePitch) const
   {
      for ( int i = 0; i < x.get_size(); i++ )
         y.idct( pWorking + i, x.get_size(), pSource + i, nSourcePitch );
      for ( int i = 0; i < y.get_size(); i++ )
         x.idct( pCoefficients + i * nCoefficientsPitch, 1, pWorking + i * x.get_size(), 1 );
   }
};

}

// don't ask
extern "C" Functions::Memset memset8_mmx;
extern "C" Functions::Memset memset8_isse;
extern "C" Functions::Memset memset8_3dnow;

extern "C" Functions::Copy copy8_mmx;
extern "C" Functions::Copy copy8_isse;
extern "C" Functions::Copy copy8_3dnow;

extern "C" Functions::Prefetch prefetch_c;
extern "C" Functions::Prefetch prefetch_isse;

extern "C" Functions::Emms emms_mmx;

}

#endif
