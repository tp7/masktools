#ifndef __Mt_Merge_H__
#define __Mt_Merge_H__

#include "../../common/base/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Merge {

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
                        const Byte *pSrc2, ptrdiff_t nSrc2Pitch, int nWidth, int nHeight);

Processor merge_c;
Processor merge_luma_420_c;

extern "C" Processor Merge_merge8_mmx;
extern "C" Processor Merge_merge8_isse;
extern "C" Processor Merge_merge8_3dnow;
extern "C" Processor Merge_merge8_sse2;
extern "C" Processor Merge_merge8_asse2;

extern "C" Processor Merge_merge_luma_4208_mmx;
extern "C" Processor Merge_merge_luma_4208_isse;
extern "C" Processor Merge_merge_luma_4208_3dnow;
extern "C" Processor Merge_merge_luma_4208_sse2;

class Filter : public MaskTools::Filter<InPlaceFilter>
{

   bool use_luma;
   ProcessorList<Processor> processors;
   ProcessorList<Processor> chroma_processors;

protected:

   virtual void process(int n, const Plane<Byte> &dst, int nPlane)
   {
      UNUSED(n);
      if ( use_luma && nPlane )
         chroma_processors.best_processor( constraints[nPlane] )( dst, dst.pitch(), frames[0].plane(nPlane), frames[0].plane(nPlane).pitch(), frames[1].plane(0), frames[1].plane(0).pitch(), dst.width(), dst.height() );
      else
         processors.best_processor( constraints[nPlane] )( dst, dst.pitch(), frames[0].plane(nPlane), frames[0].plane(nPlane).pitch(), frames[1].plane(nPlane), frames[1].plane(nPlane).pitch(), dst.width(), dst.height() );
   }

public:
   Filter(const Parameters &parameters) : MaskTools::Filter<InPlaceFilter>( parameters )
   {
      use_luma = parameters["luma"].toBool();

      if ( use_luma && (width_ratios[1][C] != 2 || height_ratios[1][C] != 2) )
      {
         error = "\"luma\" is unsupported in 422 and 444";
         return;
      }


      /* if "luma" is set, we force the chroma processing. Much more handy */
      if ( use_luma )
         operators[1] = operators[2] = PROCESS;

      /* no need to change U/V default processing, because of in place filter */

      /* add the processors */
      processors.push_back( Filtering::Processor<Processor>( merge_c, Constraint( CPU_NONE, 1, 1, 1, 1 ), 0 ) );
      processors.push_back( Filtering::Processor<Processor>( Merge_merge8_mmx, Constraint( CPU_MMX, 8, 1, 1, 1 ), 2 ) );
      processors.push_back( Filtering::Processor<Processor>( Merge_merge8_isse, Constraint( CPU_ISSE, 8, 1, 1, 1 ), 3 ) );
      processors.push_back( Filtering::Processor<Processor>( Merge_merge8_3dnow, Constraint( CPU_3DNOW, 8, 1, 1, 1 ), 4 ) );
      processors.push_back( Filtering::Processor<Processor>( Merge_merge8_sse2, Constraint( CPU_SSE2, 8, 1, 1, 1 ), 5 ) );
      processors.push_back( Filtering::Processor<Processor>( Merge_merge8_asse2, Constraint( CPU_SSE2, 8, 1, 16, 16 ), 6 ) );

      /* add the chroma processors */
      chroma_processors.push_back( Filtering::Processor<Processor>( merge_luma_420_c, Constraint( CPU_NONE, 1, 1, 1, 1 ), 0 ) );
      chroma_processors.push_back( Filtering::Processor<Processor>( Merge_merge_luma_4208_mmx, Constraint( CPU_MMX, 8, 1, 1, 1 ), 1 ) );
      chroma_processors.push_back( Filtering::Processor<Processor>( Merge_merge_luma_4208_isse, Constraint( CPU_ISSE, 8, 1, 1, 1 ), 2 ) );
      chroma_processors.push_back( Filtering::Processor<Processor>( Merge_merge_luma_4208_3dnow, Constraint( CPU_3DNOW, 8, 1, 1, 1 ), 3 ) );
      chroma_processors.push_back( Filtering::Processor<Processor>( Merge_merge_luma_4208_sse2, Constraint( CPU_SSE2, 8, 1, 1, 1 ), 4 ) );
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