#ifndef __Mt_Lutspa_H__
#define __Mt_Lutspa_H__

#include "../../../common/base/filter.h"
#include "../../../../common/parser/parser.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Lut { namespace Coordinate {

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, int nWidth, int nHeight, const Byte *lut);

Processor lut_c;

class Filter : public MaskTools::Filter<InPlaceFilter>
{
   Byte *luts[3];

protected:
   virtual void process(int n, const Plane<Byte> &dst, int nPlane)
   {
      UNUSED(n);
      lut_c( dst, dst.pitch(), dst.width(), dst.height(), luts[nPlane] );
   }

public:
   Filter(const Parameters &parameters) : MaskTools::Filter<InPlaceFilter>( parameters )
   {
      static const char *expr_strs[] = { "yExpr", "uExpr", "vExpr" };
      bool is_relative = parameters["relative"].toBool();
      bool is_biased = parameters["biased"].toBool();

      if (parameters["mode"].is_defined())
      {
         if (parameters["mode"].toString() == "absolute") is_relative = false, is_biased = false;
         else if (parameters["mode"].toString() == "relative inclusive") is_relative = true, is_biased = false;
         else if (parameters["mode"].toString() == "relative closed") is_relative = true, is_biased = false;
         else if (parameters["mode"].toString() == "relative exclusive") is_relative = true, is_biased = true;
         else if (parameters["mode"].toString() == "relative opened") is_relative = true, is_biased = true;
         else if (parameters["mode"].toString() == "relative") is_relative = true, is_biased = true;
      }

      Parser::Parser parser = Parser::getDefaultParser().addSymbol(Parser::Symbol::X).addSymbol(Parser::Symbol::Y);

      /* compute the luts */
      for ( int i = 0; i < 3; i++ )
      {
         const int w = i ? nCoreWidthUV : nCoreWidth;
         const int h = i ? nCoreHeightUV : nCoreHeight;
         if ( parameters[expr_strs[i]].is_defined() ) 
            parser.parse(parameters[expr_strs[i]].toString(), " ");
         else
            parser.parse(parameters["expr"].toString(), " ");

         Parser::Context ctx(parser.getExpression());

         if ( !ctx.check() )
         {
            error = "invalid expression in the lut";
            return;
         }

         luts[i] = new Byte[w*h];

         for ( int x = 0; x < w; x++ )
            for ( int y = 0; y < h; y++ )
               luts[i][x+y*w] = is_relative ? ctx.compute_byte(x * 1.0 / ((is_biased || w < 2) ? w : w - 1), y * 1.0 / ((is_biased || h < 2) ? h : h - 1)) : ctx.compute_byte(x, y);
      }
   }

   ~Filter()
   {
      for ( int i = 0; i < 3; i++ )
         delete[] luts[i];
   }

   InputConfiguration &input_configuration() const { return InPlaceOneFrame(); }

   static Signature filter_signature()
   {
      Signature signature = "mt_lutspa";

      signature.add(Parameter(TYPE_CLIP, ""));
      signature.add(Parameter(String("relative"), "mode"));
      signature.add(Parameter(Value(true), "relative"));
      signature.add(Parameter(Value(true), "biased"));
      signature.add(Parameter(String("x"), "expr"));
      signature.add(Parameter(String("x"), "yExpr"));
      signature.add(Parameter(String("x"), "uExpr"));
      signature.add(Parameter(String("x"), "vExpr"));

      return add_defaults( signature );
   }
};

} } } } }

#endif