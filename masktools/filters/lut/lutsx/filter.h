#ifndef __Mt_Lutsx_H__
#define __Mt_Lutsx_H__

#include "../../../common/base/filter.h"
#include "../../../../common/parser/parser.h"

#include "../functions.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Lut { namespace SpatialExtended {

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch, const Byte *pSrc2, ptrdiff_t nSrc2Pitch, const Byte *lut, const int *pCoordinates, int nCoordinates, int nWidth, int nHeight, const String &mode1, const String &mode2);

extern Processor *processors_array[NUM_MODES][NUM_MODES];

class Filter : public MaskTools::Filter<InPlaceFilter>
{
   Byte *luts[3];

   int *pCoordinates;
   int nCoordinates;

   ProcessorList<Processor> processors;
   String mode1, mode2;

   void FillCoordinates(const String &coordinates)
   {
      std::list<Parser::Symbol> coeffs = Parser::getDefaultParser().parse( coordinates, " (),;." ).getExpression();
      nCoordinates = coeffs.size();
      pCoordinates = new int[nCoordinates];
      int i = 0;

      while ( !coeffs.empty() )
      {
         pCoordinates[i++] = int( coeffs.front().getValue(0, 0, 0) );
         coeffs.pop_front();
      }
   }

protected:
   virtual void process(int n, const Plane<Byte> &dst, int nPlane)
   {
      UNUSED(n);
      processors.best_processor( constraints[nPlane] )( dst, dst.pitch(), frames[0].plane(nPlane), frames[0].plane(nPlane).pitch(), frames[1].plane(nPlane), frames[1].plane(nPlane).pitch(), luts[nPlane], pCoordinates, nCoordinates, dst.width(), dst.height(), mode1, mode2 );
   }

public:
   Filter(const Parameters &parameters) : MaskTools::Filter<InPlaceFilter>( parameters )
   {
      static const char *expr_strs[] = { "yExpr", "uExpr", "vExpr" };

      luts[0] = new Byte[ 256 * 256 * 256 ];
      luts[1] = new Byte[ 256 * 256 * 256 ];
      luts[2] = new Byte[ 256 * 256 * 256 ];

      Parser::Parser parser = Parser::getDefaultParser().addSymbol(Parser::Symbol::X).addSymbol(Parser::Symbol::Y).addSymbol(Parser::Symbol::Z);

      /* compute the luts */
      for ( int i = 0; i < 3; i++ )
      {
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

         for ( int x = 0; x < 256; x++ )
            for ( int y = 0; y < 256; y++ )
               for ( int z = 0; z < 256; z++ )
                  luts[i][(z<<16)+(x<<8)+y] = ctx.compute_byte(x, y, z);
      }

      /* get the pixels list */
      FillCoordinates( parameters["pixels"].toString() );

      /* choose the mode */
      mode1 = parameters["mode"].toString();
      mode2 = parameters["mode2"].toString();
      processors.push_back( processors_array[ ModeToInt( mode1 ) ][ ModeToInt( mode2 ) ] );
   }

   InputConfiguration &input_configuration() const { return InPlaceThreeFrame(); }

   static Signature filter_signature()
   {
      Signature signature = "mt_lutsx";

      signature.add( Parameter( TYPE_CLIP, "" ) );
      signature.add( Parameter( TYPE_CLIP, "" ) );
      signature.add( Parameter( TYPE_CLIP, "" ) );
      signature.add( Parameter( String( "average" ), "mode" ) );
      signature.add( Parameter( String( "none" ), "mode2" ) );
      signature.add( Parameter( String( "" ), "pixels" ) );
      signature.add( Parameter( String( "y" ), "expr" ) );
      signature.add( Parameter( String( "y" ), "yExpr" ) );
      signature.add( Parameter( String( "y" ), "uExpr" ) );
      signature.add( Parameter( String( "y" ), "vExpr" ) );

      return add_defaults( signature );
   }
};

} } } } }

#endif