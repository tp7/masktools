#ifndef __Mt_Lut16_H__
#define __Mt_Lut16_H__

#include "../../../common/base/filter.h"
#include "../../../../common/parser/parser.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Lut { namespace Single16bit {

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, int nWidth, int nHeight, const Word lut[65536]);

Processor lut16_c_interleaved;
Processor lut16_c_stacked;

class Lut16 : public MaskTools::Filter
{
   Word luts[3][65536];
   Processor* processor;

protected:
   virtual void process(int n, const Plane<Byte> &dst, int nPlane)
   {
      UNUSED(n);
      processor(dst, dst.pitch(), dst.width(), dst.height(), luts[nPlane]);
   }

public:
   Lut16(const Parameters &parameters) : MaskTools::Filter( parameters, FilterProcessingType::INPLACE )
   {
      static const char *expr_strs[] = { "yExpr", "uExpr", "vExpr" };

      Parser::Parser parser = Parser::getDefaultParser().addSymbol(Parser::Symbol::X);

      /* compute the luts */
      for ( int i = 0; i < 3; i++ )
      {
          if (operators[i] != PROCESS) {
              continue;
          }

          if (parameters[expr_strs[i]].undefinedOrEmptyString() && parameters["expr"].undefinedOrEmptyString()) {
              operators[i] = NONE; //inplace
              continue;
          }

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

          for ( int x = 0; x < 65536; x++ )
              luts[i][x] = ctx.compute_word(x, 0.0f);
      }

      if (parameters["stacked"].toBool()) {
          processor = lut16_c_stacked;
      } else {
          processor = lut16_c_interleaved;
      }
   }

   InputConfiguration &input_configuration() const { return InPlaceOneFrame(); }

   static Signature filter_signature()
   {
      Signature signature = "mt_lut16";

      signature.add(Parameter(TYPE_CLIP, ""));
      signature.add(Parameter(String("x"), "expr"));
      signature.add(Parameter(String("x"), "yExpr"));
      signature.add(Parameter(String("x"), "uExpr"));
      signature.add(Parameter(String("x"), "vExpr"));
      signature.add(Parameter(false, "stacked"));

      return add_defaults( signature );
   }
};


} } } } }

#endif