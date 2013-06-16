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
