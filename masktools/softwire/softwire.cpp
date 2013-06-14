#include "softwire.h"

using namespace SoftWire;

namespace Filtering { namespace SoftWire {

;


Convolution::Convolution(int nWidth, int nHeight, int nLeft, int nTop, int nRight, int nBottom) :
nWidth( nWidth ), nHeight( nHeight ), nLeft( nLeft ), nRight( nRight ), nTop( nTop ), nBottom( nBottom )
{
}

Coordinates Convolution::get_absolute_coordinates(int nX, int nY, int nDX, int nDY)
{
   int x = nX + nDX;
   int y = nY + nDY;

   if ( x < 0 )
      x = 0;

   if ( y < 0 )
      y = 0;

   if ( x >= nWidth )
      x = nWidth - 1;

   if ( y >= nHeight )
      y = nHeight - 1;

   return Coordinates( x, y );
}

Coordinates Convolution::get_relative_coordinates(int nX, int nY, int nDX, int nDY)
{
   Coordinates result = get_absolute_coordinates( nX, nY, nDX, nDY );

   result.first -= nX;
   result.second -= nY;

   return result;
}

ConvolutionAccessor::ConvolutionAccessor(const Convolution &convolution, const SoftWire::OperandREG &line0,
                                         const SoftWire::OperandREG &pitch) : Conv

} }