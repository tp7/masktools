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
   Avisynth2x::Filter<Invert::Filter>::create( env );
   Avisynth2x::Filter<Binarize::Filter>::create( env );
   Avisynth2x::Filter<Morphologic::Inflate::Filter>::create( env );
   Avisynth2x::Filter<Morphologic::Deflate::Filter>::create( env );
   Avisynth2x::Filter<Morphologic::Inpand::Filter>::create( env );
   Avisynth2x::Filter<Morphologic::Expand::Filter>::create( env );
   Avisynth2x::Filter<Lut::Single::Filter>::create( env );
   Avisynth2x::Filter<Lut::Dual::Filter>::create( env );
   Avisynth2x::Filter<Lut::Trial::Filter>::create( env );
   Avisynth2x::Filter<Lut::Spatial::Filter>::create( env );
   Avisynth2x::Filter<Lut::Frame::Filter>::create( env );
   Avisynth2x::Filter<Lut::SpatialExtended::Filter>::create( env );
   Avisynth2x::Filter<Lut::Coordinate::Filter>::create( env );
   Avisynth2x::Filter<Merge::Filter>::create( env );
   Avisynth2x::Filter<Logic::Filter>::create( env );
   Avisynth2x::Filter<Convolution::Filter>::create( env );
   Avisynth2x::Filter<Blur::Mapped::Filter>::create( env );
   Avisynth2x::Filter<Gradient::Filter>::create( env );
   Avisynth2x::Filter<Support::MakeDiff::Filter>::create( env );
   Avisynth2x::Filter<Support::Average::Filter>::create( env );
   Avisynth2x::Filter<Support::AddDiff::Filter>::create( env );
   Avisynth2x::Filter<Support::Clamp::Filter>::create( env );
   Avisynth2x::Filter<Mask::Motion::Filter>::create( env );
   Avisynth2x::Filter<Mask::Edge::Filter>::create( env );
   Avisynth2x::Filter<Mask::Hysteresis::Filter>::create( env );
   MaskTools::Avs2x::Helpers::DeclareHelpers(env);

   return("MaskTools: a set of tools to work with masks");
}
