#include "filter.h"
#include "../../../common/functions/functions.h"

using namespace Filtering;
using namespace Filtering::Functions;
using namespace Filtering::Constraints;

namespace MaskTools {

template<class ParentFilter>
Filter::Filter(const Parameters &parameters) :
ParentFilter( parameters ),
Y( parameters["Y"] ),
U( parameters["U"] ),
V( parameters["V"] ),
nXOffset( parameters["offx"] ),
nYOffset( parameters["offy"] ),
nCoreWidth( parameters["w"] ),
nCoreHeight( parameters["h"] ),
{
   if ( nXOffset < 0 || nXOffset > nWidth  ) nXOffset = 0;
   if ( nYOffset < 0 || nYOffset > nHeight ) nYOffset = 0;
   if ( nXOffset + nCoreWidth  > nWidth  || nCoreWidth  < 0 ) nCoreWidth  = nWidth - nXOffset;
   if ( nYOffset + nCoreHeight > nHeight || nCoreHeight < 0 ) nCoreHeight = nHeight - nYOffset;

   /* chroma offsets and box */
   nXOffsetUV = nXOffset / 2;
   nYOffsetUV = nYOffset / 2;
   nCoreWidthUV = (nXOffset + nCoreWidth) / 2 - nXOffsetUV;
   nCoreHeightUV = (nYOffset + nCoreHeight) / 2 - nYOffsetUV;

   /* default processors */
   /* memsets */
   memsets.push_back(Processor<Memset>(&memset_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
   memsets.push_back(Processor<Memset>(&memset8_mmx, Constraint(CPU_MMX, 8, 1, 1, 1), 1));
   memsets.push_back(Processor<Memset>(&memset8_isse, Constraint(CPU_ISSE, 8, 1, 1, 1), 2));
   memsets.push_back(Processor<Memset>(&memset8_3dnow, Constraint(CPU_3DNOW, 8, 1, 1, 1), 3));

   /* copy */
   copies.push_back(Processor<Copy>(&copy_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
   copies.push_back(Processor<Copy>(&copy8_mmx, Constraint(CPU_MMX, 8, 1, 1, 1), 1));
   copies.push_back(Processor<Copy>(&copy8_isse, Constraint(CPU_ISSE, 8, 1, 1, 1), 2));
   copies.push_back(Processor<Copy>(&copy8_3dnow, Constraint(CPU_3DNOW, 8, 1, 1, 1), 3));

   /* prefetches */
   //prefetches.push_back(Processor<Prefetch>(&prefetch_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
   //prefetches.push_back(Processor<Prefetch>(&prefetch_isse, Constraint(CPU_ISSE, 1, 1, 1, 1), 1));

   /* none */
}

template
void Frame<Byte> Filter<Filter::ProcessFrame(Frame &dst, CFrame &src1, CFrame &src2, CFrame &src3) 
{
   Plane dstp;
   CPlane srcp1, srcp2, srcp3;

   /* TODO : correct SSE2 alignement check for multiclip functions, once there is SSE2 functions */

   /* Y */
   dstp = dst.y().offset_plane(nXOffset, nYOffset);
   srcp1 = src1.y().offset_plane(nXOffset, nYOffset);
   srcp2 = src2.y().offset_plane(nXOffset, nYOffset);
   srcp3 = src3.y().offset_plane(nXOffset, nYOffset);

   Constraint constraint(flags, nWidth, nHeight, dstp.plane(), dstp.pitch());

   switch ( Mode(Y) )
   {
   case NONE: break;
   case COPY: copies.best_processor(constraint)(dstp.plane(), dstp.pitch(), srcp1.plane(), srcp1.pitch(), nWidth, nHeight); break;
   case COPY_SECOND: copies.best_processor(constraint)(dstp.plane(), dstp.pitch(), srcp2.plane(), srcp2.pitch(), nWidth, nHeight); break;
   case COPY_THIRD: copies.best_processor(constraint)(dstp.plane(), dstp.pitch(), srcp3.plane(), srcp3.pitch(), nWidth, nHeight); break;
   case MEMSET: memsets.best_processor(constraint)(dstp.plane(), dstp.pitch(), nWidth, nHeight, Y.value()); break;
   case PROCESS: ProcessY(processors.best_processor(constraint), dstp, srcp1, srcp2, srcp3); break;
   }

   /* U */
   dstp = dst.u().offset_plane(nXOffsetUV, nYOffsetUV);
   srcp1 = src1.u().offset_plane(nXOffsetUV, nYOffsetUV);
   srcp2 = src2.u().offset_plane(nXOffsetUV, nYOffsetUV);
   srcp3 = src3.u().offset_plane(nXOffsetUV, nYOffsetUV);

   constraint = Constraint(flags, nWidthUV, nHeightUV, dstp.plane(), dstp.pitch());

   switch ( Mode(U) )
   {
   case NONE: break;
   case COPY: copies.best_processor(constraint)(dstp.plane(), dstp.pitch(), srcp1.plane(), srcp1.pitch(), nWidthUV, nHeightUV); break;
   case COPY_SECOND: copies.best_processor(constraint)(dstp.plane(), dstp.pitch(), srcp2.plane(), srcp2.pitch(), nWidthUV, nHeightUV); break;
   case COPY_THIRD: copies.best_processor(constraint)(dstp.plane(), dstp.pitch(), srcp3.plane(), srcp3.pitch(), nWidthUV, nHeightUV); break;
   case MEMSET: memsets.best_processor(constraint)(dstp.plane(), dstp.pitch(), nWidthUV, nHeightUV, U.value()); break;
   case PROCESS: ProcessU(processors.best_processor(constraint), dstp, srcp1, srcp2, srcp3); break;
   }

   /* V */
   dstp = dst.v().offset_plane(nXOffsetUV, nYOffsetUV);
   srcp1 = src1.v().offset_plane(nXOffsetUV, nYOffsetUV);
   srcp2 = src2.v().offset_plane(nXOffsetUV, nYOffsetUV);
   srcp3 = src3.v().offset_plane(nXOffsetUV, nYOffsetUV);

   constraint = Constraint(flags, nWidthUV, nHeightUV, dstp.plane(), dstp.pitch());

   switch ( Mode(V) )
   {
   case NONE: break;
   case COPY: copies.best_processor(constraint)(dstp.plane(), dstp.pitch(), srcp1.plane(), srcp1.pitch(), nWidthUV, nHeightUV); break;
   case COPY_SECOND: copies.best_processor(constraint)(dstp.plane(), dstp.pitch(), srcp2.plane(), srcp2.pitch(), nWidthUV, nHeightUV); break;
   case COPY_THIRD: copies.best_processor(constraint)(dstp.plane(), dstp.pitch(), srcp3.plane(), srcp3.pitch(), nWidthUV, nHeightUV); break;
   case MEMSET: memsets.best_processor(constraint)(dstp.plane(), dstp.pitch(), nWidthUV, nHeightUV, V.value()); break;
   case PROCESS: ProcessV(processors.best_processor(constraint), dstp, srcp1, srcp2, srcp3); break;
   }

   /* todo : remove that, replace by nasm function */
   __asm { emms }
}

}
