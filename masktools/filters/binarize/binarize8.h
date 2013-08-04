#ifndef __Mt_Binarize_H__
#define __Mt_Binarize_H__

#include "../../common/base/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Binarize {

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, Byte nThreshold, int nWidth, int nHeight);

#define DEFINE_PROCESSOR(name) \
   extern Processor *binarize_##name##_c; \
   extern Processor *binarize_##name##_sse2; \
   extern Processor *binarize_##name##_asse2

DEFINE_PROCESSOR(upper);
DEFINE_PROCESSOR(lower);
DEFINE_PROCESSOR(0_x);
DEFINE_PROCESSOR(t_x);
DEFINE_PROCESSOR(x_0);
DEFINE_PROCESSOR(x_t);
DEFINE_PROCESSOR(t_0);
DEFINE_PROCESSOR(0_t);
DEFINE_PROCESSOR(x_255);
DEFINE_PROCESSOR(t_255);
DEFINE_PROCESSOR(255_x);
DEFINE_PROCESSOR(255_t);

#undef DEFINE_PROCESSOR

class Binarize : public MaskTools::Filter
{
   Byte nThreshold;
   ProcessorList<Processor> processors;

protected:
   virtual void process(int n, const Plane<Byte> &dst, int nPlane)
   {
      UNUSED(n);
      processors.best_processor( constraints[nPlane] )( dst, dst.pitch(), nThreshold, dst.width(), dst.height() );
   }

   bool isMode(const char *mode) {
       return parameters["mode"].is_defined() && parameters["mode"].toString() == mode;
   }

public:
   Binarize(const Parameters &parameters) : MaskTools::Filter(parameters, FilterProcessingType::INPLACE)
   {
      nThreshold = convert<Byte, int>( parameters["threshold"].toInt() );

#define SET_MODE(mode) \
      processors.push_back( Filtering::Processor<Processor>( binarize_##mode##_c, Constraint( CPU_NONE, 1, 1, 1, 1 ), 0 ) ); \
      processors.push_back( Filtering::Processor<Processor>( binarize_##mode##_sse2, Constraint( CPU_SSE2 , 1, 1, 1, 1 ), 1 ) ); \
      processors.push_back( Filtering::Processor<Processor>( binarize_##mode##_asse2, Constraint( CPU_SSE2 , 1, 1, 16, 16 ), 2 ) ); 

      if (isMode("0 x")) { SET_MODE(0_x); }
      else if (isMode("t x")) { SET_MODE(t_x); }
      else if (isMode("x 0")) { SET_MODE(x_0); }
      else if (isMode("x t")) { SET_MODE(x_t); }
      else if (isMode("t 0")) { SET_MODE(t_0); }
      else if (isMode("0 t")) { SET_MODE(0_t); }
      else if (isMode("x 255")) { SET_MODE(x_255); }
      else if (isMode("t 255")) { SET_MODE(t_255); }
      else if (isMode("255 x")) { SET_MODE(255_x); }
      else if (isMode("255 t")) { SET_MODE(255_t); }
      else if ((parameters["mode"].is_defined() && (parameters["mode"].toString() == "upper" || parameters["mode"].toString() == "0 255")) || (!parameters["mode"].is_defined() && parameters["upper"].toBool())) {
         SET_MODE(upper);
      } else {
         SET_MODE(lower);
      }

#undef SET_MODE
   }

   InputConfiguration &input_configuration() const { return InPlaceOneFrame(); }

   static Signature filter_signature()
   {
      Signature signature = "mt_binarize";

      signature.add( Parameter( TYPE_CLIP, "" ) );
      signature.add( Parameter( 128, "threshold" ) );
      signature.add( Parameter( false, "upper" ) );
      signature.add( Parameter( String("lower"), "mode" ) );

      return add_defaults( signature );
   }
};

} } } } // namespace Binarize, Filter, MaskTools, Filtering

#endif