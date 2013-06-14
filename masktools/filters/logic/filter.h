#ifndef __Mt_Logic_H__
#define __Mt_Logic_H__

#include "../../common/base/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Logic {

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch, int nWidth, int nHeight, Byte nThresholdDestination, Byte nThresholdSource);

#define DEFINE_PROCESSOR_NO_MMX(name) \
   extern Processor *name##_c; \
   extern "C" Processor Logic_##name##8_mmx; \
   extern "C" Processor Logic_##name##8_isse; \
   extern "C" Processor Logic_##name##8_3dnow; \
   extern "C" Processor Logic_##name##8_sse2; \
   extern "C" Processor Logic_##name##8_asse2

#define DEFINE_PROCESSOR(name) \
   DEFINE_PROCESSOR_NO_MMX(name); \
   extern "C" Processor Logic_##name##8_mmx

DEFINE_PROCESSOR(and);
DEFINE_PROCESSOR(or);
DEFINE_PROCESSOR(andn);
DEFINE_PROCESSOR(xor);
DEFINE_PROCESSOR_NO_MMX(max);
DEFINE_PROCESSOR_NO_MMX(min);

#define DEFINE_TRIPLE(mode) \
DEFINE_PROCESSOR_NO_MMX(mode); \
DEFINE_PROCESSOR_NO_MMX(mode##add); \
DEFINE_PROCESSOR_NO_MMX(mode##sub)

#define DEFINE_NINE(mode) \
DEFINE_TRIPLE(mode); \
DEFINE_TRIPLE(add##mode); \
DEFINE_TRIPLE(sub##mode)

DEFINE_NINE(min);
DEFINE_NINE(max);

#undef DEFINE_TRIPLE
#undef DEFINE_NINE
#undef DEFINE_PROCESSOR_NO_MMX
#undef DEFINE_PROCESSOR

class Filter : public MaskTools::Filter<InPlaceFilter>
{

   ProcessorList<Processor> processors;
   Byte nThresholdDestination, nThresholdSource;

protected:

   virtual void process(int n, const Plane<Byte> &dst, int nPlane)
   {
      UNUSED(n);
      processors.best_processor( constraints[nPlane] )( dst, dst.pitch(), frames[0].plane(nPlane), frames[0].plane(nPlane).pitch(), dst.width(), dst.height(), nThresholdDestination, nThresholdSource );
   }

public:
   Filter(const Parameters &parameters) : MaskTools::Filter<InPlaceFilter>( parameters )
   {

#define SET_MODE_NO_MMX(mode) \
   do { \
      processors.push_back( Filtering::Processor<Processor>( mode##_c, Constraint( CPU_NONE, 1, 1, 1, 1 ), 0 ) ); \
      processors.push_back( Filtering::Processor<Processor>( Logic_##mode##8_isse, Constraint( CPU_ISSE , 8, 1, 1, 1 ), 2 ) ); \
      processors.push_back( Filtering::Processor<Processor>( Logic_##mode##8_3dnow, Constraint( CPU_3DNOW , 8, 1, 1, 1 ), 3 ) ); \
      processors.push_back( Filtering::Processor<Processor>( Logic_##mode##8_sse2, Constraint( CPU_SSE2 , 8, 1, 1, 1 ), 4 ) ); \
      processors.push_back( Filtering::Processor<Processor>( Logic_##mode##8_asse2, Constraint( CPU_SSE2 , 8, 1, 16, 16 ), 5 ) ); \
   } while(0)

#define SET_MODE(mode) \
   do { \
      SET_MODE_NO_MMX(mode); \
      processors.push_back( Filtering::Processor<Processor>( Logic_##mode##8_mmx, Constraint( CPU_MMX , 8, 1, 1, 1 ), 2 ) ); \
   } while(0)

      int nTh1 = parameters["th1"].toInt();
      int nTh2 = parameters["th2"].toInt();

      if (parameters["mode"].toString() == "and")
         SET_MODE(and);
      else if (parameters["mode"].toString() == "or")
         SET_MODE(or);
      else if (parameters["mode"].toString() == "xor")
         SET_MODE(xor);
      else if (parameters["mode"].toString() == "andn")
         SET_MODE(andn);
      else
      {
         bool isDstSub = nTh1 < 0;
         bool isDstAdd = nTh1 > 0;
         bool isSrcSub = nTh2 < 0;
         bool isSrcAdd = nTh2 > 0;

         nThresholdDestination = convert<Byte, int>(abs<int>(nTh1));
         nThresholdSource = convert<Byte, int>(abs<int>(nTh2));

         if (parameters["mode"].toString() == "min")
         {
            if (isDstAdd && isSrcAdd) SET_MODE_NO_MMX(addminadd);
            else if (isDstAdd && isSrcSub) SET_MODE_NO_MMX(addminsub);
            else if (isDstSub && isSrcAdd) SET_MODE_NO_MMX(subminadd);
            else if (isDstSub && isSrcSub) SET_MODE_NO_MMX(subminsub);
            else if (isDstAdd) SET_MODE_NO_MMX(addmin);
            else if (isSrcAdd) SET_MODE_NO_MMX(minadd);
            else if (isDstSub) SET_MODE_NO_MMX(submin);
            else if (isSrcSub) SET_MODE_NO_MMX(minsub);
            else SET_MODE_NO_MMX(min);
         }
         else if (parameters["mode"].toString() == "max")
         {
            if (isDstAdd && isSrcAdd) SET_MODE_NO_MMX(addmaxadd);
            else if (isDstAdd && isSrcSub) SET_MODE_NO_MMX(addmaxsub);
            else if (isDstSub && isSrcAdd) SET_MODE_NO_MMX(submaxadd);
            else if (isDstSub && isSrcSub) SET_MODE_NO_MMX(submaxsub);
            else if (isDstAdd) SET_MODE_NO_MMX(addmax);
            else if (isSrcAdd) SET_MODE_NO_MMX(maxadd);
            else if (isDstSub) SET_MODE_NO_MMX(submax);
            else if (isSrcSub) SET_MODE_NO_MMX(maxsub);
            else SET_MODE_NO_MMX(max);
         }
         else
            error = "\"mode\" must be either \"and\", \"or\", \"xor\", \"andn\", \"min\" or \"max\"";
      }
   }

#undef SET_MODE
#undef SET_MODE_NO_MMX

   InputConfiguration &input_configuration() const { return InPlaceTwoFrame(); }

   static Signature filter_signature()
   {
      Signature signature = "mt_logic";

      signature.add(Parameter(TYPE_CLIP, ""));
      signature.add(Parameter(TYPE_CLIP, ""));
      signature.add(Parameter(String(""), "mode"));
      signature.add(Parameter(0, "th1"));
      signature.add(Parameter(0, "th2"));

      return add_defaults( signature );
   }

};


} } } }

#endif