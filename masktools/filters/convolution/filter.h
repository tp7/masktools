#ifndef __Mt_Convolution_H__
#define __Mt_Convolution_H__

#include "../../common/base/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Convolution {

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, void *, void *, void *, int nHorizontal, int nVertical, int nWidth, int nHeight);

extern Processor *convolution_f_s_c;
extern Processor *convolution_i_s_c;
extern Processor *convolution_f_m_c;
extern Processor *convolution_i_m_c;

class Filter : public MaskTools::Filter<ChildFilter>
{
   int *i_horizontal, *i_vertical;
   float *f_horizontal, *f_vertical;
   int i_total;
   float f_total;
   void *horizontal, *vertical;
   void *total;

   int nHorizontal;
   int nVertical;

   ProcessorList<Processor> processors;
protected:

   virtual void process(int n, const Plane<Byte> &dst, int nPlane)
   {
      UNUSED(n);
      processors.best_processor( constraints[nPlane] )( dst, dst.pitch(), frames[0].plane(nPlane), frames[0].plane(nPlane).pitch(), horizontal, vertical, total, nHorizontal, nVertical, dst.width(), dst.height() );
   }

public:
   Filter(const Parameters &parameters) : MaskTools::Filter<ChildFilter>( parameters )
   {
      i_vertical = i_horizontal = NULL;
      f_vertical = f_horizontal = NULL;
      vertical = horizontal = NULL;
      std::list<Parser::Symbol> hcoeffs = Parser::getDefaultParser().parse(parameters["horizontal"].toString(), " ").getExpression();
      std::list<Parser::Symbol> vcoeffs = Parser::getDefaultParser().parse(parameters["vertical"].toString(), " ").getExpression();
      nHorizontal = hcoeffs.size();
      nVertical = vcoeffs.size();
      
      /* search for float values */
      bool isFloat = false;
      isFloat |= parameters["horizontal"].toString().find(".", 0) != String::npos;
      isFloat |= parameters["vertical"].toString().find(".", 0) != String::npos;

      /* create the two arrays */
      if ( isFloat )
      {
         f_vertical = new float[nVertical];
         f_horizontal = new float[nHorizontal];
         f_total = float(parameters["total"].toFloat());
         vertical = f_vertical;
         horizontal = f_horizontal;
         total = parameters["total"].is_defined() ? &f_total : NULL;
      }
      else
      {
         i_vertical = new int[nVertical];
         i_horizontal = new int[nHorizontal];
         i_total = convert<int, Double>(parameters["total"].toFloat());
         if ( i_total == 0 )
            i_total = 1;
         vertical = i_vertical;
         horizontal = i_horizontal;
         total = parameters["total"].is_defined() ? &i_total : NULL;
      }

      /* fill them */
      for ( int i = 0; i < nHorizontal; i++ )
      {
         if ( isFloat )
            f_horizontal[i] = float(hcoeffs.front().getValue(0,0,0));
         else
            i_horizontal[i] = convert<int, Double>(hcoeffs.front().getValue(0,0,0));

         hcoeffs.pop_front();
      }
      for ( int i = 0; i < nVertical; i++ )
      {
         if ( isFloat )
            f_vertical[i] = float(vcoeffs.front().getValue(0,0,0));
         else
            i_vertical[i] = convert<int, Double>(vcoeffs.front().getValue(0,0,0));

         vcoeffs.pop_front();
      }

      /* adds the processor */
      bool isSaturate = parameters["saturate"].toBool();
      if ( isFloat )
         if ( isSaturate )
            processors.push_back(Filtering::Processor<Processor>(convolution_f_s_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
         else
            processors.push_back(Filtering::Processor<Processor>(convolution_f_m_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
      else
         if ( isSaturate )
            processors.push_back(Filtering::Processor<Processor>(convolution_i_s_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
         else
            processors.push_back(Filtering::Processor<Processor>(convolution_i_m_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));

   }

   ~Filter()
   {
      if ( i_horizontal )
         delete[] i_horizontal;
      if ( i_vertical )
         delete[] i_vertical;
      if ( f_horizontal )
         delete[] f_horizontal;
      if ( f_vertical )
         delete[] f_vertical;
   }

   InputConfiguration &input_configuration() const { return OneFrame(); }

   static Signature filter_signature()
   {
      Signature signature = "mt_convolution";

      signature.add(Parameter(TYPE_CLIP, ""));
      signature.add(Parameter(String("1 1 1"), "horizontal"));
      signature.add(Parameter(String("1 1 1"), "vertical"));
      signature.add(Parameter(true, "saturate"));
      signature.add(Parameter(1.0f, "total"));

      return add_defaults( signature );
   }
};

} } } }

#endif
