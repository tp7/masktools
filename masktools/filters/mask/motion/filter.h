#ifndef __Mt_Motionmask_H__
#define __Mt_Motionmask_H__

#include "../../../common/base/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Mask { namespace Motion {

typedef bool (Processor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nLowThreshold, int nHighThreshold, int nMotionThreshold, int nSceneChange, int nSceneChangeValue, int nWidth, int nHeight);

extern Processor *mask_c;
extern Processor *mask8_mmx;
extern Processor *mask8_isse;
extern Processor *mask8_sse2;
extern Processor *mask8_asse2;

class Filter : public MaskTools::Filter<InPlaceFilter>
{
   int nLowThresholds[3];
   int nHighThresholds[3];
   int nMotionThreshold;

   int nSceneChange;
   int nSceneChangeValue;

   ProcessorList<Processor> processors;

protected:

   virtual void process(int n, const Plane<Byte> &dst, int nPlane)
   {
      UNUSED(n);
      if ( nPlane == 0 )
         nSceneChange = processors.best_processor( constraints[nPlane] )( dst, dst.pitch(), frames[0].plane(nPlane), frames[0].plane(nPlane).pitch(), nLowThresholds[nPlane], nHighThresholds[nPlane], nMotionThreshold, 0, nSceneChangeValue, dst.width(), dst.height() ) ? 3 : 2;
      else
         processors.best_processor( constraints[nPlane] )( dst, dst.pitch(), frames[0].plane(nPlane), frames[0].plane(nPlane).pitch(), nLowThresholds[nPlane], nHighThresholds[nPlane], nMotionThreshold, nSceneChange, nSceneChangeValue, dst.width(), dst.height() );
   }

public:
   Filter(const Parameters &parameters) : MaskTools::Filter<InPlaceFilter>( parameters )
   {
      nLowThresholds[0] = clip<int, int>( parameters["thY1"].toInt(), 0, 255 );
      nLowThresholds[1] = nLowThresholds[2] = clip<int, int>( parameters["thC1"].toInt(), 0, 255 );
      nHighThresholds[0] = clip<int, int>( parameters["thY2"].toInt(), 0, 255 );
      nHighThresholds[1] = nHighThresholds[2] = clip<int, int>( parameters["thC2"].toInt(), 0, 255 );
      nMotionThreshold = clip<int, int>( parameters["thT"].toInt(), 0, 255 );
      nSceneChangeValue = clip<int, int>( parameters["scvalue"].toInt(), 0, 255 );

      /* add the processors */
      processors.push_back(Filtering::Processor<Processor>(mask_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
      processors.push_back(Filtering::Processor<Processor>(mask8_mmx, Constraint(CPU_MMX, 8, 1, 1, 1), 1));
      processors.push_back(Filtering::Processor<Processor>(mask8_isse, Constraint(CPU_ISSE, 8, 1, 1, 1), 2));
      processors.push_back(Filtering::Processor<Processor>(mask8_sse2, Constraint(CPU_SSE2, 8, 1, 1, 1), 3));
      processors.push_back(Filtering::Processor<Processor>(mask8_asse2, Constraint(CPU_SSE2, 8, 1, 16, 16), 3));
   }

   InputConfiguration &input_configuration() const { return InPlaceTemporalOneFrame(); }

   static Signature filter_signature()
   {
      Signature signature = "mt_motion";

      signature.add(Parameter(TYPE_CLIP, ""));
      signature.add(Parameter(10, "thY1"));
      signature.add(Parameter(10, "thY2"));
      signature.add(Parameter(10, "thC1"));
      signature.add(Parameter(10, "thC2"));
      signature.add(Parameter(10, "thT"));
      signature.add(Parameter(0, "sc_value"));

      return add_defaults( signature );
   }

};

} } } } }

#endif