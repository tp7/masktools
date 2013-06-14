#ifndef __Common_Utils_H__
#define __Common_Utils_H__

#include <assert.h>
#include <string>
#include <vector>
#include <list>
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <float.h>

#define NOMINMAX // no min & max macros
#include <windows.h>
#undef NOMINMAX

#pragma warning(disable:4267) // disable possible loss of data conversion
#pragma warning(disable:4127) // disable conditional expression is constant

#define UNUSED(x) x

namespace Filtering {

/* basic types definition : Byte, Word, Float, Double */
typedef char Char;
typedef short Short;
typedef unsigned char Byte;
typedef unsigned short Word;
typedef __int64 Int64;
typedef unsigned _int64 Uint64;
typedef float Float;
typedef double Double;
typedef std::string String;

/* case insensive == operator for String */
static inline bool operator==(const String &s1, const String &s2)
{
   return s1.size() == s2.size() && !_strnicmp( s1.c_str(), s2.c_str(), s1.size() );
}


/* colorspaces, planar */
typedef enum {

   COLORSPACE_NONE = 0,

   COLORSPACE_Y8,

   COLORSPACE_YV12,
   COLORSPACE_I420,
   COLORSPACE_YV16,
   COLORSPACE_I422,
   COLORSPACE_YV24,
   COLORSPACE_I444,

   COLORSPACE_COUNT,

} Colorspace;

/* log level */
typedef enum {

   LOG_ERROR = 0,
   LOG_WARNING = 1,
   LOG_DEBUG = 2,

} LogLevel;

/* default log level */
#ifdef DEBUG
#define MAX_LOG_LEVEL LOG_DEBUG
#else
#define MAX_LOG_LEVEL LOG_ERROR
#endif

static inline void init_random(int seed)
{
   srand( seed );
}

static inline Double uniform_random(Double min, Double max)
{
   return min + rand() * (max - min) / (RAND_MAX + 1);
}

static inline Double gaussian_random(Double mean, Double std)
{
   Double x1, x2, w;
   do {
      x1 = 2.0 * uniform_random(0, 1) - 1.0;
      x2 = 2.0 * uniform_random(0, 1) - 1.0;
      w = x1 * x1 + x2 * x2;
   } while ( w >= 1.0 );

   w = sqrt( (-2.0 * log( w ) ) / w );
   return x1 * w * std + mean;
}


/* debug print function */
static inline void print(LogLevel level, const char *format, ...)
{
   va_list args;
   char buf[1024];

   if ( level > MAX_LOG_LEVEL )
      return;

   va_start(args, format);
   vsprintf(buf, format, args);
   OutputDebugString(buf);
}

/* min & max */
template<typename T> T min(T a, T b) { return a < b ? a : b; }
template<typename T> T max(T a, T b) { return a > b ? a : b; }

static inline double fix(double a) { return _isnan(a) || !_finite(a) ? 0 : a; }

/* abs */
template<typename T> T abs(T x) { return x < 0 ? -x : x; }
template<> static inline Byte abs<Byte>(Byte x) { return x; } // unsigned abs = nop

/* max_value, for integer type */
template<typename T> T max_value() { return T(-1); } // unsigned only
template<> static inline int max_value<int>() { return 0x7fffffff; }
template<> static inline Short max_value<Short>() { return (1 << 15) - 1; }
template<> static inline Char max_value<Char>() { return (1 << 7) - 1; }
template<> static inline Int64 max_value<Int64>() { return 0x7FFFFFFFFFFFFFFFLL; }

/* min_value, for integer type */
template<typename T> T min_value() { return 0; } // unsigned only
template<> static inline Short min_value<Short>() { return -(1 << 15); }
template<> static inline Char min_value<Char>() { return -(1 << 7); }
template<> static inline Int64 min_value<Int64>() { return -1LL << 63LL; }

/* ceiled & floored round */
template<typename T> T ceiled(T x, T mod) { return ((x + mod - 1) / mod) * mod; }
template<typename T> T floored(T x, T mod) { return (x / mod) * mod; }

/* zero_threshold */
template<typename T> T zero_threshold() { return 1; } // integer types
template<> static inline Float zero_threshold<Float>() { return (Float)0.00001; } 
template<> static inline Double zero_threshold<Double>() { return 0.00001; } 

/* conversion from and to */
template<typename To, typename From> To convert(From x) { return To(x); }
template<> static inline Char convert<Char, Double>(Double x) { return x >= 0 ? Char(x + 0.5) : Char(x - 0.5); }
template<> static inline Byte convert<Byte, Double>(Double x) { return Byte(x + 0.5); }
template<> static inline Int64 convert<Int64, Double>(Double x) { return x >= 0 ? Int64(x + 0.5) : Int64(x - 0.5); }
template<> static inline Uint64 convert<Uint64, Double>(Double x) { return Uint64(x + 0.5); }
template<> static inline Short convert<Short, Double>(Double x) { return x >= 0 ? Short(x + 0.5) : Short(x - 0.5); }
template<> static inline int convert<int, Double>(Double x) { return x >= 0 ? int(x + 0.5) : int(x - 0.5); }

/* rounded division */
template<typename T> T rounded_division(T x, T y) { return x / y; }
template<> static inline int rounded_division<int>(int x, int y) { return x > 0 ? (x + (y >> 1)) / y : (x - (y >> 1)) / y; }
template<> static inline Int64 rounded_division<Int64>(Int64 x, Int64 y) { return x > 0 ? (x + (y >> 1)) / y : (x - (y >> 1)) / y; }

/* clip */
template<typename T, typename U> T clip(U x, U mini = U(min_value<T>()), U maxi = U(max_value<T>())) { return convert<T, U>( min<U>( maxi, max<U>( mini, x ) ) ); }

/* threshold */
template<typename T, typename U> T threshold(U x, U mini, U maxi, U bottom = U(min_value<T>()), U top = U(max_value<T>()))
{
   return convert<T, U>( x <= mini ? bottom : x > maxi ? top : x );
}

/* width & height ratios, according to the colorspace */
static const int plane_counts[COLORSPACE_COUNT] = { 0, 1, 3, 3, 3, 3, 3, 3, };

template<Colorspace C> int plane_count() { return plane_counts[C]; }

static const int width_ratios[3][COLORSPACE_COUNT] =
{
   { 0, 1, 1, 1, 1, 1, 1, 1, },
   { 0, 0, 2, 2, 2, 2, 1, 1, },
   { 0, 0, 2, 2, 2, 2, 1, 1, }, 
};

template<Colorspace C> int width_ratio(int nPlane) { return width_ratios[nPlane][C]; }

static const int height_ratios[3][COLORSPACE_COUNT] =
{
   { 0, 1, 1, 1, 1, 1, 1, 1, },
   { 0, 0, 2, 2, 1, 1, 1, 1, },
   { 0, 0, 2, 2, 1, 1, 1, 1, },
};

template<Colorspace C> int height_ratio(int nPlane) { return height_ratios[nPlane][C]; }

static const int pixel_sizes[3][COLORSPACE_COUNT] = 
{
   { 0, 1, 1, 1, 1, 1, 1, 1, },
   { 0, 0, 4, 4, 2, 2, 1, 1, },
   { 0, 0, 4, 4, 2, 2, 1, 1, },
};

template<Colorspace C> int pixel_size(int nPlane) { return pixel_sizes[C][nPlane]; }

/* matrix operations */
template<typename T>
class Vector : public std::vector<T> {
public:
   Vector() : std::vector<T>() {}
   Vector(T v1) : std::vector<T>() { push_back( v1 ); }
   Vector(T v1, T v2) : std::vector<T>() { push_back( v1 ); push_back( v2 ); }
};

template<typename T>
class Matrix : public Vector<Vector<T> >{

public:

   Matrix() : Vector<Vector<T> >() { }
   Matrix(Vector<T> v1, Vector<T> v2) : Vector<Vector<T> >( v1, v2 ) { }

   static int trigonalize(Matrix<T> &matrix, Vector<T> &v, std::list<int> &rrows, std::list<int> &rcolumns, std::vector<int> &nrows, std::vector<int> &ncolumns)
   {
      T max = -1;
      std::list<int>::iterator itr_max, itc_max; 

      if ( rrows.empty() )
         return 0;

      /* find the highest non trigonalized coefficient in the matrix */
      for ( std::list<int>::iterator itr = rrows.begin(); itr != rrows.end(); itr++ )
      {
         for ( std::list<int>::iterator itc = rcolumns.begin(); itc != rcolumns.end(); itc++ )
         {
            if ( max < abs<T>( matrix[*itr][*itc] ) )
            {
               itr_max = itr;
               itc_max = itc;
               max = abs<T>( matrix[*itr][*itc] );
            }
         }
      }

      T pivot = matrix[*itr_max][*itc_max];

      if ( abs<T>( pivot ) < zero_threshold<T>() )
         return -1;

      /* pivotize the remaining lines & columns */
      for ( std::list<int>::iterator itr = rrows.begin(); itr != rrows.end(); itr++ )
      {
         if ( itr == itr_max )
            continue;

         T factor = matrix[*itr][*itc_max];

         for ( std::list<int>::iterator itc = rcolumns.begin(); itc != rcolumns.end(); itc++ )
            matrix[*itr][*itc] -= matrix[*itr_max][*itc] * factor / pivot;

         matrix[*itr][*itc_max] = 0;
         v[*itr] -= v[*itr_max] * factor / pivot;
      }
      nrows.insert(nrows.begin(), *itr_max);
      ncolumns.insert(ncolumns.begin(), *itc_max);
      rrows.erase(itr_max);
      rcolumns.erase(itc_max);

      return trigonalize( matrix, v, rrows, rcolumns, nrows, ncolumns );
   }

   Vector<T> solve(const Vector<T> &v)
   {
      Vector<T> vv = v;

      std::list<int> rr, rc;
      std::vector<int> nr, nc;
      for (int i = 0; i < int(size()); i++ )
      {
         rr.push_back(i);
         rc.push_back(i);
      }
      Matrix<T> mat = *this;
      if ( trigonalize( mat, vv, rr, rc, nr, nc ) < 0 )
         return v;

      /* compute the result */
      Vector<T> result = v;

      for (int i = 0; i < int(v.size()); i++)
      {
         T precomputed = 0;
         for (int j = 0; j < i; j++)
            precomputed += result[nc[j]] * mat[nr[i]][nc[j]];

         result[nc[i]] = (vv[nr[i]] - precomputed) / mat[nr[i]][nc[i]];
      }

      /* */
      return result;
   }

};

/* ref counted class */
class RefCounted {

   int nRefCount;

public:

   RefCounted() : nRefCount(0) {}
   ~RefCounted() { assert( nRefCount == 0 ); }

   void add_ref() { nRefCount++; }
   void del_ref() { nRefCount--; }
   bool is_refed() const { return nRefCount > 0; }
   int get_ref() const { return nRefCount; }
   
};


/* smart pointer template */
template<class T>
class SmartPointer {

   T *p;

public:

   SmartPointer() : p(NULL) { }
   SmartPointer(T *p) : p(p ? p->copy() : NULL) { }
   SmartPointer(const SmartPointer<T> &p) : p(p.p ? p.p->copy() : NULL) { }
   ~SmartPointer()
   {
      if ( !p )
         return;

      p->del_ref();

      if ( !p->is_refed() )
         delete p;
   }
   SmartPointer &operator =(const SmartPointer<T> &p)
   {
      if ( this->p )
      {
         this->p->del_ref();

         if ( !this->p->is_refed() )
            delete this->p;
      }

      this->p = p.p ? p.p->copy() : NULL;

      return *this;
   }
   operator T*() const { return p; }
   T *operator->() const { assert( p ); return p; }
};

} // namespace Filtering

#endif
