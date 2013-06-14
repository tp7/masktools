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

   /* converting to const */
   //Plane(const Plane<const Byte> &plane) : pPixel(plane), nPitch(plane.pitch()), nWidth(plane.width()), nHeight(plane.height())
   //{
   //   assert( sizeof( Byte ) == sizeof( T ) );
   //}
   Plane(const Plane<const Word> &plane) : pPixel(plane), nPitch(plane.pitch()), nWidth(plane.width()), nHeight(plane.height())
   {
      assert( sizeof( Word ) == sizeof( T ) );
   }
   Plane(const Plane<Short> &plane) : pPixel(plane), nPitch(plane.pitch()), nWidth(plane.width()), nHeight(plane.height())
   {
      assert( sizeof( Short ) == sizeof( T ) );
   }
   Plane(const Plane<int> &plane) : pPixel(plane), nPitch(plane.pitch()), nWidth(plane.width()), nHeight(plane.height())
   {
      assert( sizeof( int ) == sizeof( T ) );
   }
   Plane(const Plane<Byte> &plane) : pPixel(plane), nPitch(plane.pitch()), nWidth(plane.width()), nHeight(plane.height())
   {
      assert( sizeof( Byte ) == sizeof( T ) );
   }
   Plane(const Plane<const Float> &plane) : pPixel(plane), nPitch(plane.pitch()), nWidth(plane.width()), nHeight(plane.height())
   {
      assert( sizeof( Float ) == sizeof( T ) );
   }
   Plane(const Plane<const Double> &plane) : pPlane(plane), nPitch(plane.pitch()), nWidth(plane.width()), nHeight(plane.height())
   {
      assert( sizeof( Double ) == sizeof( T ) );
   }

   operator T*() const { return pPixel; }
   ptrdiff_t pitch() const { return nPitch; }
   int width() const { return nWidth; }
   int height() const { return nHeight; }

   Plane<T> offset(int x = 0, int y = 0, int w = 0, int h = 0) const { return Plane<T>( pPixel + x + y * nPitch, nPitch, w, h ); }
   void print() const { Filtering::print( LOG_DEBUG, "plane 0x%x (%ix%i:%i) t%i\n", pPixel, nWidth, nHeight, nPitch, GetCurrentThreadId() ); }

};

template<class T>
class Pixel
{
   const Plane<T> &plane;

   int nRow;
   int nColumn;
   T *pPixel;

public:

   Pixel(const Plane<T> &plane, int x = 0, int y = 0) : plane(plane), pPixel(plane + y * plane.pitch()), nRow(y), nColumn(x) {}
   Pixel(const Pixel<T> &pixel, int x = 0, int y = 0) : plane(pixel.plane), pPixel(pixel.pPixel + y * plane.pitch()), nRow(pixel.nRow + y), nColumn(pixel.nColumn + x) {}

   Pixel<T> &operator++(int) { pPixel += plane.pitch(); nRow++; return *this; }
   Pixel<T> &operator--(int) { pPixel -= plane.pitch(); nRow--; return *this; }
   Pixel<T> &operator+=(int y) { pPixel += y * plane.pitch(); nRow += y; return *this; }
   Pixel<T> &operator-=(int y) { pPixel -= y * plane.pitch(); nRow -= y; return *this; }

   Pixel<T> &operator=(int y) { pPixel = static_cast<T*>( plane ) + y * plane.pitch(); nRow = y; nColumn = 0; return *this; }

   int width() const { return plane.width(); }
   int height() const { return plane.height(); }
   ptrdiff_t pitch() const { return plane.pitch(); }
   operator Plane<T>() const { return plane; }
   operator T*() const { return pPixel; }

   T &operator()(int x = 0, int y = 0) const { return pPixel[x + nColumn + y * plane.pitch()]; }
   T &operator[](int x) const { return pPixel[x + nColumn]; }
   T &operator*() const { return pPixel[nColumn]; }

   bool in_left(int x = 0) const { return nColumn + x >= 0; }
   bool in_right(int x = 0) const { return nColumn + x < plane.width(); }
   bool in_top(int y = 0) const { return nRow + y >= 0; }
   bool in_bottom(int y = 0) const { return nRow + y < plane.height(); }

   bool in(int x = 0, int y = 0) const { return in_left( x ) && in_right( x ) && in_top( y ) && in_bottom( y ); }
};

} // namespace Filtering

#endif

