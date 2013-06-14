#ifndef __Common_Avs2x_Clip_H__
#define __Common_Avs2x_Clip_H__

#include "../common/common.h"

#if defined(FILTER_AVS_25)
#include "avisynth-2_5.h"
#elif defined(FILTER_AVS_26)
#include <windows.h>
#include "avisynth-2_6.h"
#else
#error FILTER_AVS_2x not defined
#endif

namespace Filtering { namespace Avisynth2x {

/* plane conversion */
template<typename T> Plane<T> ConvertTo(const PVideoFrame& frame, int nPlane);

template<> static inline Plane<Byte> ConvertTo<Byte>(const PVideoFrame &frame, int nPlane)
{
   return Plane<Byte>( frame->GetWritePtr( nPlane ), frame->GetPitch( nPlane ), frame->GetRowSize( nPlane ), frame->GetHeight( nPlane ) );
}
template<> static inline Plane<const Byte> ConvertTo<const Byte>(const PVideoFrame &frame, int nPlane)
{
   return Plane<const Byte>( frame->GetReadPtr( nPlane ), frame->GetPitch( nPlane ), frame->GetRowSize( nPlane ), frame->GetHeight( nPlane ) );
}

#if defined(FILTER_AVS_25)
template<typename T> Plane<T> ConvertInterleavedTo(const PVideoFrame& frame, int nPlane);

template<> static inline Plane<Byte> ConvertInterleavedTo<Byte>(const PVideoFrame &frame, int nPlane)
{
   const int nWidth = frame->GetRowSize() / (nPlane ? 2 : 1);
   const int nHeight = frame->GetHeight();
   const ptrdiff_t nPitch = frame->GetPitch();
   return Plane<Byte>( frame->GetWritePtr() + (nPlane == 2 ? nPitch * 3 / 4 : nPlane == 1 ? nPitch / 2 : 0), nPitch, nWidth, nHeight );
}

template<> static inline Plane<const Byte> ConvertInterleavedTo<const Byte>(const PVideoFrame &frame, int nPlane)
{
   const int nWidth = frame->GetRowSize() / (nPlane ? 2 : 1);
   const int nHeight = frame->GetHeight();
   const ptrdiff_t nPitch = frame->GetPitch();
   return Plane<const Byte>( frame->GetReadPtr() + (nPlane == 2 ? nPitch * 3 / 4 : nPlane == 1 ? nPitch / 2 : 0), nPitch, nWidth, nHeight );
}
#endif

/* colorspace conversion */
Colorspace AVSColorspaceToColorspace(int pixel_type)
{
   switch ( pixel_type )
   {
   case VideoInfo::CS_I420: return COLORSPACE_I420;
   case VideoInfo::CS_YV12: return COLORSPACE_YV12;
#if defined(FILTER_AVS_26)
   case VideoInfo::CS_YV16: return COLORSPACE_YV16;
   case VideoInfo::CS_YV24: return COLORSPACE_YV24;
   case VideoInfo::CS_Y8  : return COLORSPACE_Y8;
#else if defined(FILTER_AVS_25)
   case VideoInfo::CS_YUY2: return COLORSPACE_YV16;
#endif

   default: return COLORSPACE_NONE;
   }
}

static const int PlaneOrder[] = { PLANAR_Y, PLANAR_U, PLANAR_V };

/* clip class, adapted to avs25 */
class Clip : public Filtering::Clip {

   ::PClip pclip;
   IScriptEnvironment *env;

   std::vector<PVideoFrame> frames;

public:

   Clip(const ::PClip &pclip) : pclip(pclip), env(NULL)
   {
      VideoInfo vi = pclip->GetVideoInfo();
      nWidth = vi.width;
      nHeight = vi.height;
      nFrames = vi.num_frames;
      C = AVSColorspaceToColorspace( vi.pixel_type );
   }
   virtual ~Clip()
   {
   }

   template<typename T> Frame<T> ConvertTo(const PVideoFrame& frame)
   {
      Plane<T> planes[3];
#if defined(FILTER_AVS_25)
      if ( C == COLORSPACE_YV16 )
      {
         for ( int i = 0; i < plane_counts[C]; i++ )
            planes[i] = Avisynth2x::ConvertInterleavedTo<T>( frame, i );
      }
      else
#endif
      {
         for ( int i = 0; i < plane_counts[C]; i++ )
            planes[i] = Avisynth2x::ConvertTo<T>( frame, PlaneOrder[i] ); 
      }

      return plane_counts[C] == 1 ? Frame<T>( planes[0] ) : Frame<T>( planes, C );
   }

   virtual Frame<Byte> get_frame(int n)
   {
      PVideoFrame frame;
      frame = pclip->GetFrame( n ,env );
      frames.push_back( frame );
      return ConvertTo<Byte>( frame );
   }
   virtual Frame<const Byte> get_const_frame(int n)
   {
      PVideoFrame frame;
      frame = pclip->GetFrame( clip<int>(n, 0, nFrames - 1), env );
      frames.push_back( frame );
      return ConvertTo<const Byte>( frame );
   }
   virtual void release_frames()
   {
      frames.clear();
   }
   void set_env(IScriptEnvironment *env) { this->env = env; }

};

} } // namespace Avisynth2x, Common

#endif
