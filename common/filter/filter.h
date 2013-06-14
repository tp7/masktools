#ifndef __Common_Filter_H__
#define __Common_Filter_H__

#include "../params/params.h"

namespace Filtering {

class Filter {

protected:

   ClipArray childs;
   Parameters parameters;

   int nWidth, nHeight;
   Colorspace C;

   String error;
   CpuFlags flags;

public:

   Filter(const Parameters &parameters) : parameters(parameters), flags(Functions::get_cpu_flags())
   {
      for ( int i = 0; i < int(parameters.size()); i++ )
         if ( Type( parameters[i] ) == TYPE_CLIP )
            childs.push_back( Value( parameters[i] ) );

      assert( childs.size() );

      nWidth = childs[0]->width();
      nHeight = childs[0]->height();
      C = childs[0]->colorspace();
   }
   ~Filter()
   {
   }

   ClipArray &get_childs() { return childs; }

   virtual Frame<Byte> get_frame(int n, const Frame<Byte> &output_frame) = 0;

   String get_error() const { return error; }
   bool is_error() const { return !error.empty(); }

};

class InPlaceFilter : public Filter {

public:
   InPlaceFilter(const Parameters &parameters) : Filter(parameters) { }

   static bool is_in_place() { return true; }
};

class ChildFilter : public Filter {

public:
   ChildFilter(const Parameters &parameters) : Filter(parameters) {}

   static bool is_in_place() { return false; }
};

} // namespace Filtering

#endif
