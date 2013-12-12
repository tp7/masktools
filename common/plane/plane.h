#ifndef __Common_Plane_H__
#define __Common_Plane_H__

#include "../utils/utils.h"

namespace Filtering {

template<typename T>
class Plane {

protected:

   T *pPixel;
   ptrdiff_t nPitch;
   int nWidth;
   int nHeight;

public:

   Plane() : pPixel(NULL), nPitch(0), nWidth(0), nHeight(0) {}
   Plane(T *pPixel, ptrdiff_t nPitch, int nWidth, int nHeight) : pPixel(pPixel), nPitch(nPitch), nWidth(nWidth), nHeight(nHeight) { }

   Plane(const Plane<const T> &plane) : pPixel(plane.data()), nPitch(plane.pitch()), nWidth(plane.width()), nHeight(plane.height())
   {
   }

   T* data() const { return pPixel; }
   ptrdiff_t pitch() const { return nPitch; }
   int width() const { return nWidth; }
   int height() const { return nHeight; }

   Plane<T> offset(int x = 0, int y = 0, int w = 0, int h = 0) const { return Plane<T>( pPixel + x + y * nPitch, nPitch, w, h ); }
   void print() const { Filtering::print( LOG_DEBUG, "plane 0x%x (%ix%i:%i) t%i\n", pPixel, nWidth, nHeight, nPitch, GetCurrentThreadId() ); }

};

} // namespace Filtering

#endif

