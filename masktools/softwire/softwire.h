#ifndef __Mt_Softwire_H__
#define __Mt_Softwire_H__

#include "../common/common.h"

#include <softwire/softwire.hpp>

namespace Filtering { namespace SoftWire {

;

typedef std::pair<int, int> Coordinates;

class Convolution {

   int nLeft, nRight;
   int nTop, nBottom;
   int nWidth, nHeight;

public:

   Convolution(int nWidth, int nHeight, int nLeft, int nTop, int nRight, int nBottom);

   Coordinates get_absolute_coordinates(int nX, int nY, int nDX, int nDY);
   Coordinates get_relative_coordinates(int nX, int nY, int nDX, int nDY);


};

class ConvolutionAccessor : public Convolution, public SoftWire::Assembler {

   int nX, nY;

   std::vector<SoftWire::OperandREG>   lines;
   SoftWire::OperandREF                pitch;

public:

   ConvolutionAccessor(const Convolution &convolution, const SoftWire::OperandREG &line0,
                       const SoftWire::OperandREG &pitch);

   ConvolutionAccessor(const Convolution &convolution, const SoftWire::OperandREG &line0,
                       const SoftWire::OperandREG &line1, const SoftWire::OperandREG &pitch);

   ConvolutionAccessor(const Convolution &convolution, const SoftWire::OperandREG &line0,
                       const SoftWire::OperandREG &line1, const SoftWire::OperandREG &line2,
                       const SoftWire::OperandREG &pitch);

   SoftWire::OperandREF get_load_reference(int nX, int nY);
   void next_pixels(int nPixels);
   void next_row();


};

} } // namespace SoftWire, Filtering

#endif

