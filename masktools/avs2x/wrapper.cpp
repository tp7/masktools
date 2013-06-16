//#include "wrapper.h"

#include "../filters/binarize/filter.h"
#include "../filters/invert/filter.h"
#include "../filters/logic/filter.h"
#include "../filters/merge/filter.h"
#include "../filters/convolution/filter.h"
#include "../filters/lut/lut/filter.h"
#include "../filters/lut/luts/filter.h"
#include "../filters/lut/lutf/filter.h"
#include "../filters/lut/lutxy/filter.h"
#include "../filters/lut/lutxyz/filter.h"
#include "../filters/lut/lutsx/filter.h"
#include "../filters/lut/lutspa/filter.h"
#include "../filters/mask/edge/filter.h"
#include "../filters/mask/motion/filter.h"
#include "../filters/mask/hysteresis/filter.h"
#include "../filters/morphologic/expand/filter.h"
#include "../filters/morphologic/inpand/filter.h"
#include "../filters/morphologic/inflate/filter.h"
#include "../filters/morphologic/deflate/filter.h"
#include "../filters/blur/mapped/filter.h"
#include "../filters/gradient/filter.h"
//
#include "../filters/support/adddiff/filter.h"
#include "../filters/support/makediff/filter.h"
#include "../filters/support/average/filter.h"
#include "../filters/support/clamp/filter.h"

#include "../../avs2x/avs2x.h"
#include "../helpers/avs2x/helpers_avs2x.h"

using namespace Filtering;
using namespace Filtering::MaskTools::Filters;

extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(IScriptEnvironment* env)
{
   Avisynth2x::Filter<Invert::Invert>::create( env );
   Avisynth2x::Filter<Binarize::Binarize>::create( env );
   Avisynth2x::Filter<Morphologic::Inflate::Inflate>::create( env );
   Avisynth2x::Filter<Morphologic::Deflate::Deflate>::create( env );
   Avisynth2x::Filter<Morphologic::Inpand::Inpand>::create( env );
   Avisynth2x::Filter<Morphologic::Expand::Expand>::create( env );
   Avisynth2x::Filter<Lut::Single::Lut>::create( env );
   Avisynth2x::Filter<Lut::Dual::Lutxy>::create( env );
   Avisynth2x::Filter<Lut::Trial::Lutxyz>::create( env );
   Avisynth2x::Filter<Lut::Spatial::Luts>::create( env );
   Avisynth2x::Filter<Lut::Frame::Lutf>::create( env );
   Avisynth2x::Filter<Lut::SpatialExtended::Lutsx>::create( env );
   Avisynth2x::Filter<Lut::Coordinate::Lutspa>::create( env );
   Avisynth2x::Filter<Merge::Merge>::create( env );
   Avisynth2x::Filter<Logic::Logic>::create( env );
   Avisynth2x::Filter<Convolution::Convolution>::create( env );
   Avisynth2x::Filter<Blur::Mapped::MappedBlur>::create( env );
   Avisynth2x::Filter<Gradient::Gradient>::create( env );
   Avisynth2x::Filter<Support::MakeDiff::MakeDiff>::create( env );
   Avisynth2x::Filter<Support::Average::Average>::create( env );
   Avisynth2x::Filter<Support::AddDiff::AddDiff>::create( env );
   Avisynth2x::Filter<Support::Clamp::Clamp>::create( env );
   Avisynth2x::Filter<Mask::Motion::MotionMask>::create( env );
   Avisynth2x::Filter<Mask::Edge::EdgeMask>::create( env );
   Avisynth2x::Filter<Mask::Hysteresis::Hysteresis>::create( env );
   MaskTools::Avs2x::Helpers::DeclareHelpers(env);

   return("MaskTools: a set of tools to work with masks");
}
