#ifndef __Mt_Gradient_H__
#define __Mt_Gradient_H__

#include "../../common/base/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Gradient {

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, const Byte *pRef, ptrdiff_t nRefPitch, int nWidth, int nHeight, int nX, int nY, int nMinimum, int nMaximum, int nPrecision);

extern Processor *sad_c;

class Filter : public MaskTools::Filter<ChildFilter>
{

   ProcessorList<Processor> processors;
   int nX[3];
   int nY[3];
   int nMinimum;
   int nMaximum;
   int nPrecision;

protected:

   virtual void process(int n, const Plane<Byte> &dst, int nPlane)
   {
      UNUSED(n);
      processors.best_processor( constraints[nPlane] )( dst, dst.pitch(), frames[0].plane(nPlane), frames[0].plane(nPlane).pitch(), frames[1].plane(nPlane), frames[1].plane(nPlane).pitch(), dst.width(), dst.height(), nX[nPlane], nY[nPlane], nMinimum, nMaximum, nPrecision );
   }

public:
   Filter(const Parameters &parameters) : MaskTools::Filter<ChildFilter>( parameters )
   {
      /* add the processors */
      if ( parameters["distorsion"].toString() == "sad" )
      {
         processors.push_back(Filtering::Processor<Processor>(sad_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
         nMinimum = parameters["min"].is_defined() ? parameters["min"].toInt() : 0;
         nMaximum = parameters["max"].is_defined() ? parameters["max"].toInt() : 65535;
      }
      else 
      {
         error = "unknown distorsion";
         return;
      }

      nX[0] = parameters["size_x"].is_defined() ? clip<int, int>(parameters["size_x"].toInt(), 0, nCoreWidth - 1) : nCoreWidth / 2;
      nY[0] = parameters["size_y"].is_defined() ? clip<int, int>(parameters["size_y"].toInt(), 0, nCoreHeight - 1) : nCoreHeight / 2;
      nPrecision = parameters["precision"].toInt();

      for ( int i = 1; i < plane_counts[C]; i++ )
      {
         nX[i] = nX[0] / width_ratios[i][C];
         nY[i] = nY[0] / height_ratios[i][C];
      }
   }

   InputConfiguration &input_configuration() const { return TwoFrame(); }

   static Signature filter_signature()
   {
      Signature signature = "mt_gradient";

      signature.add(Parameter(TYPE_CLIP, ""));
      signature.add(Parameter(TYPE_CLIP, ""));
      signature.add(Parameter(String("sad"), "distorsion"));
      signature.add(Parameter(0, "size_x"));
      signature.add(Parameter(0, "size_y"));
      signature.add(Parameter(0, "min"));
      signature.add(Parameter(65535, "max"));
      signature.add(Parameter(1, "precision"));

      return add_defaults( signature );
   }

};


} } } }

#endif