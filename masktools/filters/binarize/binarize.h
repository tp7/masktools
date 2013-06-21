#ifndef __Mt_Binarize_H__
#define __Mt_Binarize_H__

#include "../../common/base/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Binarize {

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, Byte nThreshold, int nWidth, int nHeight);

#define DEFINE_PROCESSOR(name) \
   extern Processor *name##_c; \
   extern Processor *name##_sse2; \
   extern Processor *name##_asse2

DEFINE_PROCESSOR(upper);
DEFINE_PROCESSOR(lower);
DEFINE_PROCESSOR(binarize_0_x);
DEFINE_PROCESSOR(binarize_t_x);
DEFINE_PROCESSOR(binarize_x_0);
DEFINE_PROCESSOR(binarize_x_t);
DEFINE_PROCESSOR(binarize_t_0);
DEFINE_PROCESSOR(binarize_0_t);
DEFINE_PROCESSOR(binarize_x_255);
DEFINE_PROCESSOR(binarize_t_255);
DEFINE_PROCESSOR(binarize_255_x);
DEFINE_PROCESSOR(binarize_255_t);

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

public:
   Binarize(const Parameters &parameters) : MaskTools::Filter(parameters, FilterProcessingType::INPLACE)
   {
      nThreshold = convert<Byte, int>( parameters["threshold"].toInt() );

#define SET_MODE(mode) \
   do { \
      processors.push_back( Filtering::Processor<Processor>( mode##_c, Constraint( CPU_NONE, 1, 1, 1, 1 ), 0 ) ); \
      processors.push_back( Filtering::Processor<Processor>( mode##_sse2, Constraint( CPU_SSE2 , 1, 1, 1, 1 ), 1 ) ); \
      processors.push_back( Filtering::Processor<Processor>( mode##_asse2, Constraint( CPU_SSE2 , 1, 1, 16, 16 ), 2 ) ); \
   } while(0)


#define IF_(mode, a, b) if (parameters["mode"].is_defined() && parameters["mode"].toString() == #a" "#b) { mode(binarize_##a##_##b); }

      IF_(SET_MODE, 0, x)
      else IF_(SET_MODE, t, x)
      else IF_(SET_MODE, x, 0)
      else IF_(SET_MODE, x, t)
      else IF_(SET_MODE, t, 0)
      else IF_(SET_MODE, 0, t)
      else IF_(SET_MODE, x, 255)
      else IF_(SET_MODE, t, 255)
      else IF_(SET_MODE, 255, x)
      else IF_(SET_MODE, 255, t)
      else if ((parameters["mode"].is_defined() && (parameters["mode"].toString() == "upper" || parameters["mode"].toString() == "0 255")) || (!parameters["mode"].is_defined() && parameters["upper"].toBool()))
         SET_MODE(upper);
      else
         SET_MODE(lower);

#undef IF_SET_MODE
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