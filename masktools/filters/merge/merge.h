#ifndef __Mt_Merge8_H__
#define __Mt_Merge8_H__

#include "../../common/base/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Merge {

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
                        const Byte *pSrc2, ptrdiff_t nSrc2Pitch, int nWidth, int nHeight);

Processor merge_c;
Processor merge_luma_420_c;

extern Processor *merge_sse2;
extern Processor *merge_asse2;
extern Processor *merge_luma_420_sse2;
extern Processor *merge_luma_420_asse2;


class Merge : public MaskTools::Filter
{

   bool use_luma;
   ProcessorList<Processor> processors;
   ProcessorList<Processor> chroma_processors;

protected:

    virtual void process(int n, const Plane<Byte> &dst, int nPlane)
    {
        UNUSED(n);
        if ( use_luma && nPlane ) {
            if (width_ratios[1][C] == 2) {
                chroma_processors.best_processor( constraints[nPlane] )( dst, dst.pitch(), frames[0].plane(nPlane), frames[0].plane(nPlane).pitch(), frames[1].plane(0), frames[1].plane(0).pitch(), dst.width(), dst.height() );
            } else {
                processors.best_processor( constraints[nPlane] )( dst, dst.pitch(), frames[0].plane(nPlane), frames[0].plane(nPlane).pitch(), frames[1].plane(0), frames[1].plane(0).pitch(), dst.width(), dst.height() );
            }
        }
        else
            processors.best_processor( constraints[nPlane] )( dst, dst.pitch(), frames[0].plane(nPlane), frames[0].plane(nPlane).pitch(), frames[1].plane(nPlane), frames[1].plane(nPlane).pitch(), dst.width(), dst.height() );
    }

public:
   Merge(const Parameters &parameters) : MaskTools::Filter( parameters, FilterProcessingType::INPLACE )
   {
      use_luma = parameters["luma"].toBool();

      if (use_luma) {
          if ((width_ratios[1][C] != 2 || height_ratios[1][C] != 2) && (width_ratios[1][C] != 1 || height_ratios[1][C] != 1)) {
              error = "\"luma\" is unsupported in 422";
              return;
          }
          auto c1 = childs[0]->colorspace();
          auto c2 = childs[1]->colorspace();
          if ((width_ratios[1][c1] != width_ratios[1][c2]) || (height_ratios[1][c1] != height_ratios[1][c2])) {
              error = "clips should have identical colorspace";
              return;
          }

          /* if "luma" is set, we force the chroma processing. Much more handy */
          operators[1] = operators[2] = PROCESS;
          /* no need to change U/V default processing, because of in place filter */
      }

      /* add the processors */
      processors.push_back( Filtering::Processor<Processor>( merge_c, Constraint( CPU_NONE, 1, 1, 1, 1 ), 0 ) );
      processors.push_back( Filtering::Processor<Processor>( merge_sse2, Constraint( CPU_SSE2, 1, 1, 1, 1 ), 1 ) );
      processors.push_back( Filtering::Processor<Processor>( merge_asse2, Constraint( CPU_SSE2, 1, 1, 16, 16 ), 2 ) );

      /* add the chroma processors */
      chroma_processors.push_back( Filtering::Processor<Processor>( merge_luma_420_c, Constraint( CPU_NONE, 1, 1, 1, 1 ), 0 ) );
      chroma_processors.push_back( Filtering::Processor<Processor>( merge_luma_420_sse2, Constraint( CPU_SSE2, 1, 1, 1, 1 ), 1 ) );
      chroma_processors.push_back( Filtering::Processor<Processor>( merge_luma_420_asse2, Constraint( CPU_SSE2, 1, 1, 16, 16 ), 2) );
   }

   InputConfiguration &input_configuration() const { return InPlaceThreeFrame(); }

   static Signature filter_signature()
   {
      Signature signature = "mt_merge";

      signature.add( Parameter( TYPE_CLIP, "" ) );
      signature.add( Parameter( TYPE_CLIP, "" ) );
      signature.add( Parameter( TYPE_CLIP, "" ) );
      signature.add( Parameter( false, "luma" ) );

      return add_defaults( signature );
   }
};

} } } } // namespace Merge, Filters, MaskTools, Filtering

#endif