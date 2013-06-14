#ifndef __Mt_Clip_H__
#define __Mt_Clip_H__

#include "../common.h"
#include <vector>

namespace Filtering { namespace MaskTools {

class Input {

   int nOffset;
   int nIndex;

public:

   Input(int nIndex = 0, int nOffset = 0) : nIndex(nIndex), nOffset(nOffset) {}

   int index() const { return nIndex; }
   int offset() const { return nOffset; }

};

class InputConfiguration : public std::vector<Input> {

public:

   InputConfiguration() {}
   InputConfiguration(const Input &input) { push_back( input ); }
   InputConfiguration(const Input &input1, const Input &input2) { push_back( input1 ); push_back( input2 ); }
   InputConfiguration(const Input &input1, const Input &input2, const Input &input3) { push_back( input1 ); push_back( input2 ); push_back( input3 ); }

};

/* thanks ticpp for that tips */
InputConfiguration &InPlaceOneFrame();
InputConfiguration &OneFrame();
InputConfiguration &InPlaceTwoFrame();
InputConfiguration &TwoFrame();
InputConfiguration &InPlaceThreeFrame();
InputConfiguration &InPlaceTemporalOneFrame();
InputConfiguration &TemporalOneFrame();

} } // namespace MaskTools, Filtering

#endif
