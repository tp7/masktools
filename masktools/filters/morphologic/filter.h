#ifndef __Mt_MorphologicFilter_H__
#define __Mt_MorphologicFilter_H__

#include "../../common/base/filter.h"
#include "../../../common/parser/parser.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic {

typedef void (Processor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nMaxDeviation, const int *pCoordinates, int nCoordinates, int nWidth, int nHeight);

class Filter : public MaskTools::Filter<ChildFilter>
{
   int nMaxDeviations[3];
   int *pCoordinates, nCoordinates;

   Filter(const Filter &filter);

protected:

   ProcessorList<Processor> processors;

   virtual void process(int n, const Plane<Byte> &dst, int nPlane)
   {
      UNUSED(n);
      processors.best_processor( constraints[nPlane] )( dst, dst.pitch(), frames[0].plane(nPlane), frames[0].plane(nPlane).pitch(), nMaxDeviations[nPlane], pCoordinates, nCoordinates, dst.width(), dst.height() );
   }

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

public:
   Filter(const Parameters &parameters) : MaskTools::Filter<ChildFilter>( parameters ), pCoordinates( NULL ), nCoordinates( 0 )
   {
      nMaxDeviations[0] = clip<int, int>( parameters["thY"].toInt(), 0, 255 );
      nMaxDeviations[1] = clip<int, int>( parameters["thC"].toInt(), 0, 255 );
      nMaxDeviations[2] = clip<int, int>( parameters["thC"].toInt(), 0, 255 );
   }

   ~Filter()
   {
      if ( pCoordinates )
         delete[] pCoordinates;
      pCoordinates = NULL;
   }

   InputConfiguration &input_configuration() const { return OneFrame(); }
};

} } } } // namespace Morphologic, Filters, MaskTools, Filtering

#endif