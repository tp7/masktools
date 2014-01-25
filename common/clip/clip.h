#ifndef __Common_Clip_H__
#define __Common_Clip_H__

#include "../frame/frame.h"
#include <memory>
#if defined(FILTER_AVS_25)
#include "avisynth-2_5.h"
#elif defined(FILTER_AVS_26)
#include <windows.h>
#include <avisynth.h>
#else
#error FILTER_AVS_2x not defined
#endif

namespace Filtering { 

class Clip {

protected:

   int nFrames;
   int nWidth, nHeight;
   Colorspace C;

public:

   Clip() : nFrames(-1), nWidth(-1), nHeight(-1), C(COLORSPACE_NONE) { }
   Clip(int nWidth, int nHeight, int nFrames, Colorspace colorspace) : nFrames(nFrames), nWidth(nWidth), nHeight(nHeight), C(colorspace) { }
   Clip(const Clip *clip) : nFrames(clip ? clip->frame_count() : -1), nWidth(clip ? clip->width() : -1), nHeight(clip ? clip->height() : -1), C(clip ? clip->colorspace() : COLORSPACE_NONE) { }
   int width() const { return nWidth; }
   int height() const { return nHeight; }
   int frame_count() const { return nFrames; }
   Colorspace colorspace() const { return C; }

   virtual Frame<Byte> get_frame(int n, IScriptEnvironment *env) = 0;
   virtual Frame<const Byte> get_const_frame(int n, IScriptEnvironment *env) = 0;

};

typedef std::shared_ptr<Clip> PClip;
typedef std::vector<PClip> ClipArray;

} // namespace Filtering

#endif
