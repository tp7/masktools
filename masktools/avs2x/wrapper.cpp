#include "../filters/binarize/binarize.h"

#include "../filters/invert/invert.h"
#include "../filters/logic/logic.h"
#include "../filters/merge/merge.h"
#include "../filters/convolution/convolution.h"
#include "../filters/lut/lut/lut.h"
#include "../filters/lut/luts/luts.h"
#include "../filters/lut/lutf/lutf.h"
#include "../filters/lut/lutxy/lutxy.h"
#include "../filters/lut/lutxyz/lutxyz.h"
#include "../filters/lut/lutsx/lutsx.h"
#include "../filters/lut/lutspa/lutspa.h"
#include "../filters/mask/edge/edgemask.h"
#include "../filters/mask/motion/motionmask.h"
#include "../filters/mask/hysteresis/hysteresis.h"
#include "../filters/morphologic/expand/expand.h"
#include "../filters/morphologic/inpand/inpand.h"
#include "../filters/morphologic/inflate/inflate.h"
#include "../filters/morphologic/deflate/deflate.h"
#include "../filters/blur/mappedblur.h"
#include "../filters/gradient/gradient.h"
//
#include "../filters/support/adddiff/adddiff.h"
#include "../filters/support/makediff/makediff.h"
#include "../filters/support/average/average.h"
#include "../filters/support/clamp/clamp.h"

#ifdef ENABLE_16BIT_SUPPORT
#include "../filters/binarize/binarize16.h"
#include "../filters/merge/merge16.h"
#include "../filters/lut/lut16/lut16.h"
#include "../filters/logic/logic16.h"
#include "../filters/support/clamp16/clamp16.h"
#include "../filters/support/average16/average16.h"
#include "../filters/support/adddiff16/adddiff16.h"
#include "../filters/support/makediff16/makediff16.h"
#endif

#include "../../avs2x/avs2x.h"
#include "../helpers/avs2x/helpers_avs2x.h"

using namespace Filtering;
using namespace Filtering::MaskTools::Filters;

#ifdef FILTER_AVS_25
extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(IScriptEnvironment* env) {
#else
const AVS_Linkage *AVS_linkage = nullptr;

extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit3(IScriptEnvironment* env, const AVS_Linkage* const vectors) {
    AVS_linkage = vectors;
#endif

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
   Avisynth2x::Filter<Blur::MappedBlur>::create( env );
   Avisynth2x::Filter<Gradient::Gradient>::create( env );
   Avisynth2x::Filter<Support::MakeDiff::MakeDiff>::create( env );
   Avisynth2x::Filter<Support::Average::Average>::create( env );
   Avisynth2x::Filter<Support::AddDiff::AddDiff>::create( env );
   Avisynth2x::Filter<Support::Clamp::Clamp>::create( env );
   Avisynth2x::Filter<Mask::Motion::MotionMask>::create( env );
   Avisynth2x::Filter<Mask::Edge::EdgeMask>::create( env );
   Avisynth2x::Filter<Mask::Hysteresis::Hysteresis>::create( env );
   MaskTools::Avs2x::Helpers::DeclareHelpers(env);

#ifdef ENABLE_16BIT_SUPPORT
   Avisynth2x::Filter<Binarize16::Binarize16>::create( env );
   Avisynth2x::Filter<Lut::Single16bit::Lut16>::create( env );
   Avisynth2x::Filter<Merge16::Merge16>::create( env );
   Avisynth2x::Filter<Logic16::Logic16>::create( env );
   Avisynth2x::Filter<Support::Average16::Average16>::create( env );
   Avisynth2x::Filter<Support::MakeDiff16::MakeDiff16>::create( env );
   Avisynth2x::Filter<Support::AddDiff16::AddDiff16>::create( env );
   Avisynth2x::Filter<Support::Clamp16::Clamp16>::create( env );
#endif
   return("MaskTools: a set of tools to work with masks");
}
