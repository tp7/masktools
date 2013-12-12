#ifndef __Common_Frame_H__
#define __Common_Frame_H__

#include "../plane/plane.h"

namespace Filtering {

template<typename T>
class Frame {

   Plane<T> planes[3];
   Colorspace C;

public:

   Frame() {}
   Frame(const Plane<T> &plane)
   {
       C = COLORSPACE_Y8;
       planes[0] = plane;
   }

   Frame(const Plane<T> planes[3], Colorspace C) : C(C)
   {
       assert(plane_counts[C] == 3);
       for (int i = 0; i < 3; i++) {
           this->planes[i] = planes[i];
       }
   }

   Frame(const Plane<T> &y, const Plane<T> &u, const Plane<T> &v, Colorspace C) : C(C)
   {
       assert(plane_counts[C] == 3);
       planes[0] = y;
       planes[1] = u;
       planes[2] = v;
   }
   
   Frame(T* pBuffer, int nWidth, int nHeight, Colorspace C, int nPaddingWidth = 0, int nPaddingHeight = 0) : C(C)
   {
      for ( int i = 0; i < plane_counts[C]; i++ )
      {
         const int w = nWidth / width_ratios[i][C];
         const int h = nHeight / height_ratios[i][C];

         planes[i] = Plane<T>( pBuffer, w + 2 * nPaddingWidth, w + 2 * nPaddingWidth, h + 2 * nPaddingHeight );
         pBuffer += (w + 2 * nPaddingWidth) * (h + 2 * nPaddingHeight);
      }
   }

   template<class U>
   Frame(const Frame<U> &frame) : C(frame.colorspace())
   {
       for (int i = 0; i < plane_counts[C]; i++) {
           planes[i] = Plane<T>(frame.plane(i));
       }
   }

   Plane<T> plane(int nPlane) const {
       assert(nPlane < plane_counts[C]);
       return planes[nPlane];
   }

   Frame<T> offset(int x = 0, int y = 0, int w = 0, int h = 0) const
   {
       if (plane_counts[C] > 1)
       {
           w = ((x + w) / width_ratios[2][C]) * width_ratios[2][C];
           h = ((y + h) / height_ratios[2][C]) * height_ratios[2][C];
           x = (x / width_ratios[2][C]) * width_ratios[2][C];
           y = (y / height_ratios[2][C]) * height_ratios[2][C];
           w -= x;
           h -= y;
           return Frame<T>(planes[0].offset(x, y, w, h),
                           planes[1].offset(x / width_ratios[1][C], y / height_ratios[1][C], w / width_ratios[1][C], h / height_ratios[1][C]),
                           planes[2].offset(x / width_ratios[2][C], y / height_ratios[2][C], w / width_ratios[2][C], h / height_ratios[2][C]),
                           C);
       } else {
           return Frame<T>(planes[0].offset(x, y, w, h));
       }
   }

   Frame<T> select(int paddx, int paddy) const
   {
       return Frame<T>(planes[0].offset(paddx, paddy, width(0) - 2 * paddx, height(0) - 2 * paddy),
                       planes[1].offset(paddx, paddy, width(1) - 2 * paddx, height(1) - 2 * paddy),
                       planes[2].offset(paddx, paddy, width(2) - 2 * paddx, height(2) - 2 * paddy),
                       C);
   }

   int width(int nPlane) const { assert( nPlane < plane_counts[C] ); return planes[nPlane].width(); }
   int height(int nPlane) const { assert( nPlane < plane_counts[C] ); return planes[nPlane].height(); }
   Colorspace colorspace() const { return C; }
};

} // namespace Filtering

#endif
