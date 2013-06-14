// Avisynth v2.5.  Copyright 2002, 2005 Ben Rudiak-Gould et al.
// Avisynth v2.6.  Copyright 2006 Klaus Post.
// Avisynth v2.6.  Copyright 2007 Ian Brabham.
// http://www.avisynth.org

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA, or visit
// http://www.gnu.org/copyleft/gpl.html .
//
// Linking Avisynth statically or dynamically with other modules is making a
// combined work based on Avisynth.  Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.


/*
Please NOTE! This version of avisynth.h DOES NOT have any special exemption!

         While this version is under development you are fully
       constrained by the terms of the GNU General Public License.

 Any derivative software you may publish MUST include the full source code.

    Normal licence conditions will be reapplied in a future version.
*/




#ifndef __AVISYNTH_H__
#define __AVISYNTH_H__

#if defined(FILTER_AVS_26)
#define IanB
#endif

enum { AVISYNTH_INTERFACE_VERSION = 5 };


/* Define all types necessary for interfacing with avisynth.dll
   Moved from internal.h */

// Win32 API macros, notably the types BYTE, DWORD, ULONG, etc.
#include <windef.h>

// COM interface macros
#include <objbase.h>


// Raster types used by VirtualDub & Avisynth
#define in64 (__int64)(unsigned short)
typedef unsigned long	Pixel;    // this will break on 64-bit machines!
typedef unsigned long	Pixel32;
typedef unsigned char	Pixel8;
typedef long			PixCoord;
typedef long			PixDim;
typedef long			PixOffset;


/* Compiler-specific crap */

// Tell MSVC to stop precompiling here
#ifdef _MSC_VER
  #pragma hdrstop
#endif

// Set up debugging macros for MS compilers; for others, step down to the
// standard <assert.h> interface
#ifdef _MSC_VER
  #include <crtdbg.h>
#else
  #define _RPT0(a,b) ((void)0)
  #define _RPT1(a,b,c) ((void)0)
  #define _RPT2(a,b,c,d) ((void)0)
  #define _RPT3(a,b,c,d,e) ((void)0)
  #define _RPT4(a,b,c,d,e,f) ((void)0)

  #define _ASSERTE(x) assert(x)
  #include <assert.h>
#endif



// I had problems with Premiere wanting 1-byte alignment for its structures,
// so I now set the Avisynth struct alignment explicitly here.
#pragma pack(push,8)

#define FRAME_ALIGN 16
// Default frame alignment is 16 bytes, to help P4, when using SSE2

// The VideoInfo struct holds global information about a clip (i.e.
// information that does not depend on the frame number).  The GetVideoInfo
// method in IClip returns this struct.

// Audio Sample information
typedef float SFLOAT;

enum {SAMPLE_INT8  = 1<<0,
      SAMPLE_INT16 = 1<<1,
      SAMPLE_INT24 = 1<<2,    // Int24 is a very stupid thing to code, but it's supported by some hardware.
      SAMPLE_INT32 = 1<<3,
      SAMPLE_FLOAT = 1<<4};

enum {
   PLANAR_Y=1<<0,
   PLANAR_U=1<<1,
   PLANAR_V=1<<2,
   PLANAR_ALIGNED=1<<3,
   PLANAR_Y_ALIGNED=PLANAR_Y|PLANAR_ALIGNED,
   PLANAR_U_ALIGNED=PLANAR_U|PLANAR_ALIGNED,
   PLANAR_V_ALIGNED=PLANAR_V|PLANAR_ALIGNED,
#ifdef IanB
   PLANAR_A=1<<4,
   PLANAR_R=1<<5,
   PLANAR_G=1<<6,
   PLANAR_B=1<<7,
   PLANAR_A_ALIGNED=PLANAR_A|PLANAR_ALIGNED,
   PLANAR_R_ALIGNED=PLANAR_R|PLANAR_ALIGNED,
   PLANAR_G_ALIGNED=PLANAR_G|PLANAR_ALIGNED,
   PLANAR_B_ALIGNED=PLANAR_B|PLANAR_ALIGNED,
#endif
  };

class AvisynthError /* exception */ {
public:
  const char* const msg;
  AvisynthError(const char* _msg) : msg(_msg) {}
};

struct VideoInfo {
  int width, height;    // width=0 means no video
  unsigned fps_numerator, fps_denominator;
  int num_frames;
  // This is more extensible than previous versions. More properties can be added seeminglesly.

  // Colorspace properties.
/*
7<<0  Planar Width Subsampling bits
      Use (X+1) & 3 for GetPlaneWidthSubsampling
        000 => 1        YV12, YV16
        001 => 2        YV411, YUV9
        010 => reserved
        011 => 0        YV24
        1xx => reserved

1<<3  VPlaneFirst YV12, YV16, YV24, YV411, YUV9
1<<4  UPlaneFirst I420

7<<8  Planar Height Subsampling bits
      Use ((X>>8)+1) & 3 for GetPlaneHeightSubsampling
        000 => 1        YV12
        001 => 2        YUV9
        010 => reserved
        011 => 0        YV16, YV24, YV411
        1xx => reserved

7<<16 Sample resolution bits
        000 => 8
        001 => 16
        010 => 32
        011 => reserved
        1xx => reserved

Planar match mask 1111.0000.0000.0111.0000.0111.0000.0111
Planar signature  10xx.0000.0000.00xx.0000.00xx.00xx.00xx
*/
  enum {
    CS_BGR = 1<<28,
    CS_YUV = 1<<29,
    CS_INTERLEAVED = 1<<30,
    CS_PLANAR = 1<<31, // Probably should move and reserve this bit for 2.5 compatibility
#ifdef IanB
    CS_Shift_Sub_Width   =  0,
    CS_Shift_Sub_Height  =  8,
    CS_Shift_Sample_Bits = 16,

    CS_Sub_Width_Mask   = 7 << CS_Shift_Sub_Width,
    CS_Sub_Width_1      = 3 << CS_Shift_Sub_Width, // YV24
    CS_Sub_Width_2      = 0 << CS_Shift_Sub_Width, // YV12, I420, YV16
    CS_Sub_Width_4      = 1 << CS_Shift_Sub_Width, // YUV9, YV411
                               
    CS_VPlaneFirst      = 1 << 3, // YV12, YV16, YV24, YV411, YUV9
    CS_UPlaneFirst      = 1 << 4, // I420
                               
    CS_Sub_Height_Mask  = 7 << CS_Shift_Sub_Height,
    CS_Sub_Height_1     = 3 << CS_Shift_Sub_Height, // YV16, YV24, YV411
    CS_Sub_Height_2     = 0 << CS_Shift_Sub_Height, // YV12, I420
    CS_Sub_Height_4     = 1 << CS_Shift_Sub_Height, // YUV9
                               
    CS_Sample_Bits_Mask = 7 << CS_Shift_Sample_Bits,
    CS_Sample_Bits_8    = 0 << CS_Shift_Sample_Bits,
    CS_Sample_Bits_16   = 1 << CS_Shift_Sample_Bits,
    CS_Sample_Bits_32   = 2 << CS_Shift_Sample_Bits,

    CS_PLANAR_MASK      = CS_PLANAR | CS_INTERLEAVED | CS_YUV | CS_BGR | CS_Sample_Bits_Mask | CS_Sub_Height_Mask | CS_Sub_Width_Mask,
#endif
  };

  // Specific colorformats
  enum { CS_UNKNOWN = 0,
         CS_BGR24 = 1<<0 | CS_BGR | CS_INTERLEAVED,
         CS_BGR32 = 1<<1 | CS_BGR | CS_INTERLEAVED,
         CS_YUY2  = 1<<2 | CS_YUV | CS_INTERLEAVED,
#ifdef IanB
      // CS_YV12  = 1<<3  Reserved
      // CS_I420  = 1<<4  Reserved

         CS_RAW32 = 1<<5 | CS_INTERLEAVED,

// YV12 must be 0xA000008 2.5 Baked API will see all new planar as YV12
// I420 must be 0xA000010

         CS_YV24  = CS_PLANAR | CS_YUV | CS_Sample_Bits_8 | CS_VPlaneFirst | CS_Sub_Height_1 | CS_Sub_Width_1,  // YUV 4:4:4 planar
         CS_YV16  = CS_PLANAR | CS_YUV | CS_Sample_Bits_8 | CS_VPlaneFirst | CS_Sub_Height_1 | CS_Sub_Width_2,  // YUV 4:2:2 planar
         CS_YV12  = CS_PLANAR | CS_YUV | CS_Sample_Bits_8 | CS_VPlaneFirst | CS_Sub_Height_2 | CS_Sub_Width_2,  // y-v-u, 4:2:0 planar
         CS_I420  = CS_PLANAR | CS_YUV | CS_Sample_Bits_8 | CS_UPlaneFirst | CS_Sub_Height_2 | CS_Sub_Width_2,  // y-u-v, 4:2:0 planar
         CS_IYUV  = CS_I420,
         CS_YUV9  = CS_PLANAR | CS_YUV | CS_Sample_Bits_8 | CS_VPlaneFirst | CS_Sub_Height_4 | CS_Sub_Width_4,  // YUV 4:1:0 planar
         CS_YV411 = CS_PLANAR | CS_YUV | CS_Sample_Bits_8 | CS_VPlaneFirst | CS_Sub_Height_1 | CS_Sub_Width_4,  // YUV 4:1:1 planar

         CS_Y8    = CS_PLANAR | CS_INTERLEAVED | CS_YUV | CS_Sample_Bits_8,                                     // Y   4:0:0 planar

         CS_YV48  = CS_PLANAR | CS_YUV | CS_Sample_Bits_16 | CS_VPlaneFirst | CS_Sub_Height_1 | CS_Sub_Width_1, // YUV 4:4:4 16bit samples
         CS_Y16   = CS_PLANAR | CS_INTERLEAVED | CS_YUV | CS_Sample_Bits_16,                                    // Y   4:0:0 16bit samples

         CS_YV96  = CS_PLANAR | CS_YUV | CS_Sample_Bits_32 | CS_VPlaneFirst | CS_Sub_Height_1 | CS_Sub_Width_1, // YUV 4:4:4 32bit samples
         CS_Y32   = CS_PLANAR | CS_INTERLEAVED | CS_YUV | CS_Sample_Bits_32,                                    // Y   4:0:0 32bit samples

         CS_PRGB  = CS_PLANAR | CS_BGR | CS_Sample_Bits_8,                                                      // Planar RGB
         CS_RGB48 = CS_PLANAR | CS_BGR | CS_Sample_Bits_16,                                                     // Planar RGB 16bit samples
         CS_RGB96 = CS_PLANAR | CS_BGR | CS_Sample_Bits_32,                                                     // Planar RGB 32bit samples
#else
         CS_YV12  = 1<<3 | CS_YUV | CS_PLANAR,  // y-v-u, 4:2:0 planar
         CS_I420  = 1<<4 | CS_YUV | CS_PLANAR,  // y-u-v, 4:2:0 planar
         CS_IYUV  = 1<<4 | CS_YUV | CS_PLANAR,  // same as above
// New as of 2.6:
         CS_YV24  = 1<<6 | CS_YUV | CS_PLANAR,  // YUV 4:4:4 planar
         CS_YV16  = 1<<7 | CS_YUV | CS_PLANAR,  // YUV 4:2:2 planar
         CS_Y8    = 1<<8 | CS_YUV | CS_PLANAR,  // Y   4:0:0 planar
         CS_YV411 = 1<<9 | CS_YUV | CS_PLANAR,  // YUV 4:1:1 planar
//       CS_YUV9  = 1<<10| CS_YUV | CS_PLANAR,  // YUV 4:1:0 planar ::FIXME::
#endif
  };
  int pixel_type;                // changed to int as of 2.5


  int audio_samples_per_second;   // 0 means no audio
  int sample_type;                // as of 2.5
  __int64 num_audio_samples;      // changed as of 2.5
  int nchannels;                  // as of 2.5

  // Imagetype properties

  int image_type;

  enum {
    IT_BFF = 1<<0,
    IT_TFF = 1<<1,
    IT_FIELDBASED = 1<<2
  };

  // Chroma placement bits 20 -> 23  ::FIXME:: Really want a Class to support this
  enum {
    CS_UNKNOWN_CHROMA_PLACEMENT = 0 << 20,
    CS_MPEG1_CHROMA_PLACEMENT   = 1 << 20,
    CS_MPEG2_CHROMA_PLACEMENT   = 2 << 20,
    CS_YUY2_CHROMA_PLACEMENT    = 3 << 20,
    CS_TOPLEFT_CHROMA_PLACEMENT = 4 << 20
  };

  // useful functions of the above
  bool HasVideo() const { return (width!=0); }
  bool HasAudio() const { return (audio_samples_per_second!=0); }
  bool IsRGB() const { return !!(pixel_type&CS_BGR); }
  bool IsRGB24() const { return (pixel_type&CS_BGR24)==CS_BGR24; } // Clear out additional properties
  bool IsRGB32() const { return (pixel_type & CS_BGR32) == CS_BGR32 ; }
  bool IsYUV() const { return !!(pixel_type&CS_YUV ); }
  bool IsYUY2() const { return (pixel_type & CS_YUY2) == CS_YUY2; }
#ifndef IanB
  bool IsYV12() const { return ((pixel_type & CS_YV12) == CS_YV12)||((pixel_type & CS_I420) == CS_I420); }
  bool IsYV24() const { return (pixel_type & CS_YV24) == CS_YV24; }
  bool IsYV16() const { return (pixel_type & CS_YV16) == CS_YV16; }
  bool IsY8() const { return (pixel_type & CS_Y8) == CS_Y8; }
  bool IsYV411() const { return (pixel_type & CS_YV411) == CS_YV411; }

  bool IsColorSpace(int c_space) const { return ((pixel_type & c_space) == c_space); }
#else
  bool IsYV24()  const { return (pixel_type & CS_PLANAR_MASK) == CS_YV24;  }
  bool IsYV16()  const { return (pixel_type & CS_PLANAR_MASK) == CS_YV16;  }
  bool IsYV12()  const { return (pixel_type & CS_PLANAR_MASK) == CS_YV12;  }
  bool IsYV411() const { return (pixel_type & CS_PLANAR_MASK) == CS_YV411; }
//bool IsYUV9()  const { return (pixel_type & CS_PLANAR_MASK) == CS_YUV9;  }
  bool IsY8()    const { return (pixel_type & CS_PLANAR_MASK) == CS_Y8;    }

  bool IsColorSpace(int c_space) const { return IsPlanar() ? ((pixel_type & CS_PLANAR_MASK) == c_space) : ((pixel_type & c_space) == c_space); }
#endif

  bool Is(int property) const { return ((pixel_type & property)==property ); }
  bool IsPlanar() const { return !!(pixel_type & CS_PLANAR); }
  bool IsFieldBased() const { return !!(image_type & IT_FIELDBASED); }
  bool IsParityKnown() const { return ((image_type & IT_FIELDBASED)&&(image_type & (IT_BFF|IT_TFF))); }
  bool IsBFF() const { return !!(image_type & IT_BFF); }
  bool IsTFF() const { return !!(image_type & IT_TFF); }
#ifndef IanB
  bool IsVPlaneFirst() const {return IsYV16()|| IsYV24() || IsYV411() || ((pixel_type & CS_YV12) == CS_YV12); }  // Don't use this
  int BytesFromPixels(int pixels) const { return IsPlanar() ? pixels : pixels * (BitsPerPixel()>>3); }   // Will not work on planar images, but will return only luma planes
#else
  bool IsVPlaneFirst() const {return (pixel_type & (CS_VPlaneFirst | CS_UPlaneFirst)) == CS_VPlaneFirst; }
  int BytesFromPixels(int pixels) const { return IsPlanar() ? pixels << ((pixel_type>>CS_Shift_Sample_Bits) & 3) : pixels * (BitsPerPixel()>>3); }
#endif
  __int64 AudioSamplesFromFrames(__int64 frames) const { return (fps_numerator && HasVideo()) ? ((__int64)(frames) * audio_samples_per_second * fps_denominator / fps_numerator) : 0; }
  int FramesFromAudioSamples(__int64 samples) const { return (fps_denominator && HasAudio()) ? (int)((samples * (__int64)fps_numerator)/((__int64)fps_denominator * (__int64)audio_samples_per_second)) : 0; }
  __int64 AudioSamplesFromBytes(__int64 bytes) const { return HasAudio() ? bytes / BytesPerAudioSample() : 0; }
  __int64 BytesFromAudioSamples(__int64 samples) const { return samples * BytesPerAudioSample(); }
  int AudioChannels() const { return HasAudio() ? nchannels : 0; }
  int SampleType() const{ return sample_type;}
  bool IsSampleType(int testtype) const{ return !!(sample_type&testtype);}
  int SamplesPerSecond() const { return audio_samples_per_second; }
  int BytesPerAudioSample() const { return nchannels*BytesPerChannelSample();}
  void SetFieldBased(bool isfieldbased)  { if (isfieldbased) image_type|=IT_FIELDBASED; else  image_type&=~IT_FIELDBASED; }
  void Set(int property)  { image_type|=property; }
  void Clear(int property)  { image_type&=~property; }

  int GetPlaneWidthSubsampling(int plane = 0) const {  // Subsampling in bitshifts!
#ifdef IanB
    if (!plane || plane == PLANAR_Y)  // No subsampling
      return 0;
    if (IsY8())
      throw AvisynthError("Filter error: GetPlaneWidthSubsampling not available on Y8 pixel type.");
    if (plane == PLANAR_U || plane == PLANAR_V) {
      if (IsYUY2())
        return 1;
      else if (IsPlanar())
        return ((pixel_type>>CS_Shift_Sub_Width)+1) & 3;
	  else
        throw AvisynthError("Filter error: GetPlaneWidthSubsampling called with unsupported pixel type.");
	}
    throw AvisynthError("Filter error: GetPlaneWidthSubsampling called with unsupported plane.");
#else
    if (!plane || plane == PLANAR_Y)  // No subsampling
      return 0;
    if (IsYV24())
      return 0;
    if (IsYV12() || IsYV16() || IsYUY2())
      return 1;
    if (IsYV411())
      return 2;
    if (IsY8())
      throw AvisynthError("Filter error: GetPlaneWidthSubsampling not available on Y8 pixel type.");


    throw AvisynthError("Filter error: GetPlaneWidthSubsampling called with unknown pixel type.");
#endif
    return 0;
  }

  int GetPlaneHeightSubsampling(int plane = 0) const {  // Subsampling in bitshifts!
#ifdef IanB
    if (!plane || plane == PLANAR_Y)  // No subsampling
      return 0;
    if (IsY8())
      throw AvisynthError("Filter error: GetPlaneHeightSubsampling not available on Y8 pixel type.");
    if (plane == PLANAR_U || plane == PLANAR_V) {
      if (IsYUY2())
        return 0;
      else if (IsPlanar())
        return ((pixel_type>>CS_Shift_Sub_Width)+1) & 3;
	  else
        throw AvisynthError("Filter error: GetPlaneHeightSubsampling called with unsupported pixel type.");
	}
    throw AvisynthError("Filter error: GetPlaneHeightSubsampling called with supported plane.");
#else
    if (!plane || plane == PLANAR_Y)  // No subsampling
      return 0;
    if (IsYV12())
      return 1;
    if (IsYV24() || IsYV16() || IsYUY2() || IsYV411())
      return 0;
    if (IsY8())
      throw AvisynthError("Filter error: GetPlaneHeightSubsampling not available on Y8 pixel type.");

    throw AvisynthError("Filter error: GetPlaneHeightSubsampling called with unknown pixel type.");
#endif
    return 0;
  }

  int RowSize(int plane = 0) const {
    if (!plane ||  plane == PLANAR_Y) {
      return BytesFromPixels(width);
    }
    if (IsY8()) {
      return 0;
    }
    if (IsPlanar()) {
      return BytesFromPixels(width)>>GetPlaneWidthSubsampling(plane);
    }
    return BytesFromPixels(width);
  }

  int BMPSize() const {
    if (IsY8())
      return height * ((RowSize()+3) & ~3);

    if (IsPlanar()) {
      // Y plane
      int Ybytes = height * ((RowSize()+3) & ~3);
      if (IsYV12()) {
        return Ybytes + Ybytes/2;  // Legacy alignment
      }
      int UVbytes = (RowSize(PLANAR_U)+3) & ~3;
      UVbytes *= height>>GetPlaneHeightSubsampling(PLANAR_U);
      return Ybytes + UVbytes*2;
    }
    return height * ((RowSize()+3) & ~3);
  }

  int BitsPerPixel() const {
#ifndef IanB
    switch (pixel_type) {
      case CS_BGR24:
      case CS_YV24:
        return 24;
      case CS_BGR32:
        return 32;
      case CS_YUY2:
      case CS_YV16:
        return 16;
      case CS_YV12:
      case CS_I420:
      case CS_YV411:
        return 12;
//    case CS_YUV9:
//      return 9;
      case CS_Y8:
        return 8;
      default:
        return 0;
    }
#else
// Lookup Interleaved, calculate PLANAR's
    switch (pixel_type) {
      case CS_BGR24:
        return 24;
      case CS_BGR32:
        return 32;
      case CS_YUY2:
        return 16;
      case CS_Y8:
        return 8;
      case CS_Y16:
        return 16;
      case CS_Y32:
        return 32;
      default:
        if (IsPlanar()) {
          const int S = IsYUV() ? GetPlaneWidthSubsampling(PLANAR_U) + GetPlaneHeightSubsampling(PLANAR_U) : 0;
          return ( ((1<<S)+2) * (8<<((pixel_type>>CS_Shift_Sample_Bits) & 3)) ) >> S;
        }
        return 0;
    }
#endif
  }

  int BytesPerChannelSample() const {
    switch (sample_type) {
    case SAMPLE_INT8:
      return sizeof(signed char);
    case SAMPLE_INT16:
      return sizeof(signed short);
    case SAMPLE_INT24:
      return 3;
    case SAMPLE_INT32:
      return sizeof(signed int);
    case SAMPLE_FLOAT:
      return sizeof(SFLOAT);
    default:
      _ASSERTE("Sample type not recognized!");
      return 0;
    }
  }

  // useful mutator
  void SetFPS(unsigned numerator, unsigned denominator) {
	if ((numerator == 0) || (denominator == 0)) {
	  fps_numerator = 0;
	  fps_denominator = 1;
	}
	else {
	  unsigned x=numerator, y=denominator;
	  while (y) {   // find gcd
		unsigned t = x%y; x = y; y = t;
	  }
	  fps_numerator = numerator/x;
	  fps_denominator = denominator/x;
	}
  }

  // Range protected multiply-divide of FPS
  void MulDivFPS(unsigned multiplier, unsigned divisor) {
	unsigned __int64 numerator   = UInt32x32To64(fps_numerator,   multiplier);
	unsigned __int64 denominator = UInt32x32To64(fps_denominator, divisor);

	unsigned __int64 x=numerator, y=denominator;
	while (y) {   // find gcd
	  unsigned __int64 t = x%y; x = y; y = t;
	}
	numerator   /= x; // normalize
	denominator /= x;

	unsigned __int64 temp = numerator | denominator; // Just looking top bit
	unsigned u = 0;
	while (temp & 0xffffffff80000000) { // or perhaps > 16777216*2
	  temp = Int64ShrlMod32(temp, 1);
	  u++;
	}
	if (u) { // Scale to fit
	  const unsigned round = 1 << (u-1);
	  SetFPS( (unsigned)Int64ShrlMod32(numerator   + round, u),
	          (unsigned)Int64ShrlMod32(denominator + round, u) );
	}
	else {
	  fps_numerator   = (unsigned)numerator;
	  fps_denominator = (unsigned)denominator;
	}
  }

  // Test for same colorspace
  bool IsSameColorspace(const VideoInfo& vi) const {
    if (vi.pixel_type == pixel_type) return TRUE;
    if (IsYV12() && vi.IsYV12()) return TRUE;
    return FALSE;
  }

};




// VideoFrameBuffer holds information about a memory block which is used
// for video data.  For efficiency, instances of this class are not deleted
// when the refcount reaches zero; instead they're stored in a linked list
// to be reused.  The instances are deleted when the corresponding AVS
// file is closed.

class VideoFrameBuffer {
  BYTE* const data;
  const int data_size;
  // sequence_number is incremented every time the buffer is changed, so
  // that stale views can tell they're no longer valid.
  long sequence_number;

  friend class VideoFrame;
  friend class Cache;
  friend class CacheMT;  // ::FIXME:: illegal class extension
  friend class ScriptEnvironment;
  long refcount;

public:
  VideoFrameBuffer(int size);
  VideoFrameBuffer();
  ~VideoFrameBuffer();

  const BYTE* GetReadPtr() const { return data; }
  BYTE* GetWritePtr() { ++sequence_number; return data; }
  int GetDataSize() { return data_size; }
  int GetSequenceNumber() { return sequence_number; }
  int GetRefcount() { return refcount; }
};


class IClip;
class PClip;
class PVideoFrame;
class IScriptEnvironment;
class AVSValue;


// VideoFrame holds a "window" into a VideoFrameBuffer.  Operator new
// is overloaded to recycle class instances.

class VideoFrame {
  int refcount;
  VideoFrameBuffer* const vfb;
  const int offset, pitch, row_size, height, offsetU, offsetV, pitchUV;  // U&V offsets are from top of picture.
  const int row_sizeUV, heightUV;

  friend class PVideoFrame;
  void AddRef() { InterlockedIncrement((long *)&refcount); }
  void Release() { if (refcount==1) InterlockedDecrement(&vfb->refcount); InterlockedDecrement((long *)&refcount); }

  friend class ScriptEnvironment;
  friend class Cache;
  friend class CacheMT;  // ::FIXME:: illegal class extension

  VideoFrame(VideoFrameBuffer* _vfb, int _offset, int _pitch, int _row_size, int _height);
  VideoFrame(VideoFrameBuffer* _vfb, int _offset, int _pitch, int _row_size, int _height, int _offsetU, int _offsetV, int _pitchUV, int _row_sizeUV, int _heightUV);

  void* operator new(size_t size);
// TESTME: OFFSET U/V may be switched to what could be expected from AVI standard!
public:
  int GetPitch() const { return pitch; }
  int GetPitch(int plane) const { switch (plane) {case PLANAR_U: case PLANAR_V: return pitchUV;} return pitch; }
  int GetRowSize() const { return row_size; }
  int GetRowSize(int plane) const {
    switch (plane) {
    case PLANAR_U: case PLANAR_V: if (pitchUV) return row_sizeUV;  else return 0;
    case PLANAR_U_ALIGNED: case PLANAR_V_ALIGNED:
      if (pitchUV) {
        int r = (row_sizeUV+FRAME_ALIGN-1)&(~(FRAME_ALIGN-1)); // Aligned rowsize
        if (r<=pitchUV)
          return r;
        return row_sizeUV;
      } else return 0;
    case PLANAR_Y_ALIGNED:
      int r = (row_size+FRAME_ALIGN-1)&(~(FRAME_ALIGN-1)); // Aligned rowsize
      if (r<=pitch)
        return r;
      return row_size;
    }
    return row_size; }
  int GetHeight() const { return height; }
  int GetHeight(int plane) const {  switch (plane) {case PLANAR_U: case PLANAR_V: if (pitchUV) return heightUV;  return 0;} return height; }

  // generally you shouldn't use these three
  VideoFrameBuffer* GetFrameBuffer() const { return vfb; }
  int GetOffset() const { return offset; }
  int GetOffset(int plane) const { switch (plane) {case PLANAR_U: return offsetU;case PLANAR_V: return offsetV;default: return offset;}; }

  // in plugins use env->SubFrame() -- because implementation code is only available inside avisynth.dll. Doh!
  // If you use these remember to increase vfb->refcount before calling and decrement it afterwards.
  VideoFrame* Subframe(int rel_offset, int new_pitch, int new_row_size, int new_height) const;
  VideoFrame* Subframe(int rel_offset, int new_pitch, int new_row_size, int new_height, int rel_offsetU, int rel_offsetV, int pitchUV) const;


  const BYTE* GetReadPtr() const { return vfb->GetReadPtr() + offset; }
  const BYTE* GetReadPtr(int plane) const { return vfb->GetReadPtr() + GetOffset(plane); }

  bool IsWritable() const { return (refcount == 1 && vfb->refcount == 1); }

  BYTE* GetWritePtr() const {
    if (vfb->GetRefcount()>1) {
      _ASSERT(FALSE);
      //throw AvisynthError("Internal Error - refcount was more than one!");
    }
    return IsWritable() ? (vfb->GetWritePtr() + offset) : 0;
  }

  BYTE* GetWritePtr(int plane) const {
    if (plane==PLANAR_Y) {
      if (vfb->GetRefcount()>1) {
        _ASSERT(FALSE);
//        throw AvisynthError("Internal Error - refcount was more than one!");
      }
      return IsWritable() ? vfb->GetWritePtr() + GetOffset(plane) : 0;
    }
    return vfb->data + GetOffset(plane);
  }

  ~VideoFrame() { InterlockedDecrement(&vfb->refcount); }
};

enum {
  CACHE_NOTHING=0,
  CACHE_RANGE=1,
  CACHE_ALL=2,
  CACHE_AUDIO=3,
  CACHE_AUDIO_NONE=4,
  CACHE_AUDIO_AUTO=5
 };

// Base class for all filters.
class IClip {
  friend class PClip;
  friend class AVSValue;
  int refcnt;
  void AddRef() { InterlockedIncrement((long *)&refcnt); }
  void Release() { InterlockedDecrement((long *)&refcnt); if (!refcnt) delete this; }
public:
  IClip() : refcnt(0) {}

  virtual int __stdcall GetVersion() { return AVISYNTH_INTERFACE_VERSION; }

  virtual PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env) = 0;
  virtual bool __stdcall GetParity(int n) = 0;  // return field parity if field_based, else parity of first field in frame
  virtual void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) = 0;  // start and count are in samples
  virtual void __stdcall SetCacheHints(int cachehints,int frame_range) = 0 ;  // We do not pass cache requests upwards, only to the next filter.
  virtual const VideoInfo& __stdcall GetVideoInfo() = 0;
  virtual __stdcall ~IClip() {}
};


// smart pointer to IClip
class PClip {

  IClip* p;

  IClip* GetPointerWithAddRef() const { if (p) p->AddRef(); return p; }
  friend class AVSValue;
  friend class VideoFrame;

  void Init(IClip* x) {
    if (x) x->AddRef();
    p=x;
  }
  void Set(IClip* x) {
    if (x) x->AddRef();
    if (p) p->Release();
    p=x;
  }

public:
  PClip() { p = 0; }
  PClip(const PClip& x) { Init(x.p); }
  PClip(IClip* x) { Init(x); }
  void operator=(IClip* x) { Set(x); }
  void operator=(const PClip& x) { Set(x.p); }

  IClip* operator->() const { return p; }

  // useful in conditional expressions
  operator void*() const { return p; }
  bool operator!() const { return !p; }

  ~PClip() { if (p) p->Release(); }
};


// smart pointer to VideoFrame
class PVideoFrame {

  VideoFrame* p;

  void Init(VideoFrame* x) {
    if (x) x->AddRef();
    p=x;
  }
  void Set(VideoFrame* x) {
    if (x) x->AddRef();
    if (p) p->Release();
    p=x;
  }

public:
  PVideoFrame() { p = 0; }
  PVideoFrame(const PVideoFrame& x) { Init(x.p); }
  PVideoFrame(VideoFrame* x) { Init(x); }
  void operator=(VideoFrame* x) { Set(x); }
  void operator=(const PVideoFrame& x) { Set(x.p); }

  VideoFrame* operator->() const { return p; }

  // for conditional expressions
  operator void*() const { return p; }
  bool operator!() const { return !p; }

  ~PVideoFrame() { if (p) p->Release();}
};


class AVSValue {
public:

  AVSValue() { type = 'v'; }
  AVSValue(IClip* c) { type = 'c'; clip = c; if (c) c->AddRef(); }
  AVSValue(const PClip& c) { type = 'c'; clip = c.GetPointerWithAddRef(); }
  AVSValue(bool b) { type = 'b'; boolean = b; }
  AVSValue(int i) { type = 'i'; integer = i; }
//  AVSValue(__int64 l) { type = 'l'; longlong = l; }
  AVSValue(float f) { type = 'f'; floating_pt = f; }
  AVSValue(double f) { type = 'f'; floating_pt = float(f); }
  AVSValue(const char* s) { type = 's'; string = s; }
  AVSValue(const AVSValue* a, int size) { type = 'a'; array = a; array_size = (short)size; }
  AVSValue(const AVSValue& v) { Assign(&v, true); }

  ~AVSValue() { if (IsClip() && clip) clip->Release(); }
  AVSValue& operator=(const AVSValue& v) { Assign(&v, false); return *this; }

  // Note that we transparently allow 'int' to be treated as 'float'.
  // There are no int<->bool conversions, though.

  bool Defined() const { return type != 'v'; }
  bool IsClip() const { return type == 'c'; }
  bool IsBool() const { return type == 'b'; }
  bool IsInt() const { return type == 'i'; }
//  bool IsLong() const { return (type == 'l'|| type == 'i'); }
  bool IsFloat() const { return type == 'f' || type == 'i'; }
  bool IsString() const { return type == 's'; }
  bool IsArray() const { return type == 'a'; }

  PClip AsClip() const { _ASSERTE(IsClip()); return IsClip()?clip:0; }
  bool AsBool() const { _ASSERTE(IsBool()); return boolean; }
  int AsInt() const { _ASSERTE(IsInt()); return integer; }
//  int AsLong() const { _ASSERTE(IsLong()); return longlong; }
  const char* AsString() const { _ASSERTE(IsString()); return IsString()?string:0; }
  float AsFloat() const { _ASSERTE(IsFloat()); return IsInt()?integer:floating_pt; }

  bool AsBool(bool def) const { _ASSERTE(IsBool()||!Defined()); return IsBool() ? boolean : def; }
  int AsInt(int def) const { _ASSERTE(IsInt()||!Defined()); return IsInt() ? integer : def; }
  float AsFloat(double def) const { _ASSERTE(IsFloat()||!Defined()); return IsInt() ? integer : type=='f' ? floating_pt : (float)def; }
  const char* AsString(const char* def) const { _ASSERTE(IsString()||!Defined()); return IsString() ? string : def; }

  int ArraySize() const { _ASSERTE(IsArray()); return IsArray()?array_size:1; }

  const AVSValue& operator[](int index) const {
    _ASSERTE(IsArray() && index>=0 && index<array_size);
    return (IsArray() && index>=0 && index<array_size) ? array[index] : *this;
  }

private:

  short type;  // 'a'rray, 'c'lip, 'b'ool, 'i'nt, 'f'loat, 's'tring, 'v'oid, or 'l'ong
  short array_size;
  union {
    IClip* clip;
    bool boolean;
    int integer;
    float floating_pt;
    const char* string;
    const AVSValue* array;
//    __int64 longlong;
  };

  void Assign(const AVSValue* src, bool init) {
    if (src->IsClip() && src->clip)
      src->clip->AddRef();
    if (!init && IsClip() && clip)
      clip->Release();
    // make sure this copies the whole struct!
    ((__int32*)this)[0] = ((__int32*)src)[0];
    ((__int32*)this)[1] = ((__int32*)src)[1];
  }
};


// instantiable null filter
class GenericVideoFilter : public IClip {
protected:
  PClip child;
  VideoInfo vi;
public:
  GenericVideoFilter(PClip _child) : child(_child) { vi = child->GetVideoInfo(); }
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env) { return child->GetFrame(n, env); }
  void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) { child->GetAudio(buf, start, count, env); }
  const VideoInfo& __stdcall GetVideoInfo() { return vi; }
  bool __stdcall GetParity(int n) { return child->GetParity(n); }
  void __stdcall SetCacheHints(int cachehints,int frame_range) { } ;  // We do not pass cache requests upwards, only to the next filter.
};





/* Helper classes useful to plugin authors */ // But we don't export the entry points, Doh!

class AlignPlanar : public GenericVideoFilter
{
public:
  AlignPlanar(PClip _clip);
  static PClip Create(PClip clip);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
};



class FillBorder : public GenericVideoFilter
{
public:
  FillBorder(PClip _clip);
  static PClip Create(PClip clip);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
};



class ConvertAudio : public GenericVideoFilter
/**
  * Helper class to convert audio to any format
 **/
{
public:
  ConvertAudio(PClip _clip, int prefered_format);
  void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env);
  void __stdcall SetCacheHints(int cachehints,int frame_range);  // We do pass cache requests upwards, to the cache!

  static PClip Create(PClip clip, int sample_type, int prefered_type);
  static AVSValue __cdecl Create_float(AVSValue args, void*, IScriptEnvironment*);
  static AVSValue __cdecl Create_32bit(AVSValue args, void*, IScriptEnvironment*);
  static AVSValue __cdecl Create_24bit(AVSValue args, void*, IScriptEnvironment*);
  static AVSValue __cdecl Create_16bit(AVSValue args, void*, IScriptEnvironment*);
  static AVSValue __cdecl Create_8bit (AVSValue args, void*, IScriptEnvironment*);
  static AVSValue __cdecl Create_Any  (AVSValue args, void*, IScriptEnvironment*);
  virtual ~ConvertAudio();

private:
  void convertToFloat(char* inbuf, float* outbuf, char sample_type, int count);
  void convertToFloat_3DN(char* inbuf, float* outbuf, char sample_type, int count);
  void convertToFloat_SSE(char* inbuf, float* outbuf, char sample_type, int count);
  void convertToFloat_SSE2(char* inbuf, float* outbuf, char sample_type, int count);
  void convertFromFloat(float* inbuf, void* outbuf, char sample_type, int count);
  void convertFromFloat_3DN(float* inbuf, void* outbuf, char sample_type, int count);
  void convertFromFloat_SSE(float* inbuf, void* outbuf, char sample_type, int count);
  void convertFromFloat_SSE2(float* inbuf, void* outbuf, char sample_type, int count);

  __inline int Saturate_int8(float n);
  __inline short Saturate_int16(float n);
  __inline int Saturate_int24(float n);
  __inline int Saturate_int32(float n);

  char src_format;
  char dst_format;
  int src_bps;
  char *tempbuffer;
  SFLOAT *floatbuffer;
  int tempbuffer_size;
};


// For GetCPUFlags.  These are backwards-compatible with those in VirtualDub.
enum {
                    /* slowest CPU to support extension */
  CPUF_FORCE        =  0x01,   //  N/A
  CPUF_FPU          =  0x02,   //  386/486DX
  CPUF_MMX          =  0x04,   //  P55C, K6, PII
  CPUF_INTEGER_SSE  =  0x08,   //  PIII, Athlon
  CPUF_SSE          =  0x10,   //  PIII, Athlon XP/MP
  CPUF_SSE2         =  0x20,   //  PIV, Hammer
  CPUF_3DNOW        =  0x40,   //  K6-2
  CPUF_3DNOW_EXT    =  0x80,   //  Athlon
  CPUF_X86_64       =  0xA0,   //  Hammer (note: equiv. to 3DNow + SSE2, which
                               //          only Hammer will have anyway)
  CPUF_SSE3         = 0x100,   //  PIV+, Hammer
  CPUF_SSSE3        = 0x200,   //  Core 2
  CPUF_SSE4			= 0x400,   //  Penryn Also called SSE4.1
  CPUF_SSE4_2		= 0x800,   //  Penryn
};
#define MAX_INT 0x7fffffff
//#define MIN_INT -0x7fffffff  // ::FIXME:: research why this is not 0x80000000


class IClipLocalStorage;

class IScriptEnvironment {
public:
  virtual __stdcall ~IScriptEnvironment() {}

  virtual /*static*/ long __stdcall GetCPUFlags() = 0;

  virtual char* __stdcall SaveString(const char* s, int length = -1) = 0;
  virtual char* __stdcall Sprintf(const char* fmt, ...) = 0;
  // note: val is really a va_list; I hope everyone typedefs va_list to a pointer
  virtual char* __stdcall VSprintf(const char* fmt, void* val) = 0;

  __declspec(noreturn) virtual void __stdcall ThrowError(const char* fmt, ...) = 0;

  class NotFound /*exception*/ {};  // thrown by Invoke and GetVar

  typedef AVSValue (__cdecl *ApplyFunc)(AVSValue args, void* user_data, IScriptEnvironment* env);

  virtual void __stdcall AddFunction(const char* name, const char* params, ApplyFunc apply, void* user_data) = 0;
  virtual bool __stdcall FunctionExists(const char* name) = 0;
  virtual AVSValue __stdcall Invoke(const char* name, const AVSValue args, const char** arg_names=0) = 0;

  virtual AVSValue __stdcall GetVar(const char* name) = 0;
  virtual bool __stdcall SetVar(const char* name, const AVSValue& val) = 0;
  virtual bool __stdcall SetGlobalVar(const char* name, const AVSValue& val) = 0;

  virtual void __stdcall PushContext(int level=0) = 0;
  virtual void __stdcall PopContext() = 0;

  // align should be 4 or 8
  virtual PVideoFrame __stdcall NewVideoFrame(const VideoInfo& vi, int align=FRAME_ALIGN) = 0;

  virtual bool __stdcall MakeWritable(PVideoFrame* pvf) = 0;

  virtual /*static*/ void __stdcall BitBlt(BYTE* dstp, int dst_pitch, const BYTE* srcp, int src_pitch, int row_size, int height) = 0;

  typedef void (__cdecl *ShutdownFunc)(void* user_data, IScriptEnvironment* env);
  virtual void __stdcall AtExit(ShutdownFunc function, void* user_data) = 0;

  virtual void __stdcall CheckVersion(int version = AVISYNTH_INTERFACE_VERSION) = 0;

  virtual PVideoFrame __stdcall Subframe(PVideoFrame src, int rel_offset, int new_pitch, int new_row_size, int new_height) = 0;

  virtual int __stdcall SetMemoryMax(int mem) = 0;

  virtual int __stdcall SetWorkingDir(const char * newdir) = 0;

  virtual void* __stdcall ManageCache(int key, void* data) = 0;

  enum PlanarChromaAlignmentMode {
			PlanarChromaAlignmentOff,
			PlanarChromaAlignmentOn,
			PlanarChromaAlignmentTest };

  virtual bool __stdcall PlanarChromaAlignment(PlanarChromaAlignmentMode key) = 0;

  virtual PVideoFrame __stdcall SubframePlanar(PVideoFrame src, int rel_offset, int new_pitch, int new_row_size, int new_height, int rel_offsetU, int rel_offsetV, int new_pitchUV) = 0;

  virtual void __stdcall SetMTMode(int mode,int threads,bool temporary)=0;
  virtual int __stdcall  GetMTMode(bool return_nthreads)=0;

  virtual IClipLocalStorage* __stdcall AllocClipLocalStorage()=0;

  virtual void __stdcall SaveClipLocalStorage()=0;
  virtual void __stdcall RestoreClipLocalStorage()=0;
};


// avisynth.dll exports this; it's a way to use it as a library, without
// writing an AVS script or without going through AVIFile.
IScriptEnvironment* __stdcall CreateScriptEnvironment(int version = AVISYNTH_INTERFACE_VERSION);


#pragma pack(pop)

#endif //__AVISYNTH_H__
