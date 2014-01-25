#ifndef __Mt_Lutspa_H__
#define __Mt_Lutspa_H__

#include "../../../common/base/filter.h"
#include "../../../../common/parser/parser.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Lut { namespace Coordinate {

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, int nWidth, int nHeight, const Byte *lut);

Processor lut_c;

class Lutspa : public MaskTools::Filter
{
   Byte *luts[3];

protected:
    virtual void process(int n, const Plane<Byte> &dst, int nPlane, const Filtering::Frame<const Byte> frames[3], const Constraint constraints[3]) override
    {
        UNUSED(n); UNUSED(frames); UNUSED(constraints);
        Functions::copy_plane(dst.data(), dst.pitch(), luts[nPlane], dst.width(), dst.width(), dst.height());
    }

public:
   Lutspa(const Parameters &parameters) : MaskTools::Filter(parameters, FilterProcessingType::CHILD)
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
          luts[i] = nullptr;

          if (operators[i] != PROCESS) {
              continue;
          }
          
          if (parameters[expr_strs[i]].undefinedOrEmptyString() && parameters["expr"].undefinedOrEmptyString()) {
              operators[i] = Operator(MEMSET, 0); //for no real reason
              continue;
          }

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

         luts[i] = reinterpret_cast<Byte*>(_aligned_malloc(w*h, 16));

         for ( int x = 0; x < w; x++ )
            for ( int y = 0; y < h; y++ )
               luts[i][x+y*w] = is_relative ? ctx.compute_byte(x * 1.0 / ((is_biased || w < 2) ? w : w - 1), y * 1.0 / ((is_biased || h < 2) ? h : h - 1)) : ctx.compute_byte(x, y);
      }
   }

   ~Lutspa()
   {
       for (int i = 0; i < 3; i++) {
           _aligned_free(luts[i]);
       }
   }

   InputConfiguration &input_configuration() const { 
       for (int i = 0; i < 3; i++) {
           if (operators[i] == COPY) {
               return OneFrame();
           }
       }
       return InPlaceOneFrame();
   }

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