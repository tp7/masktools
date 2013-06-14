#include "functions.h"

using namespace Filtering;

void Functions::memset_c(Byte *pPlane, ptrdiff_t nPitch, int nWidth, int nHeight, Byte value)
{
   for ( int i = 0; i < nHeight; i++, pPlane += nPitch )
      memset(pPlane, value, nWidth);
}

void Functions::copy_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch,
                       int nWidth, int nHeight)
{
   for ( int i = 0; i < nHeight; i++, pDst += nDstPitch, pSrc += nSrcPitch )
      memcpy(pDst, pSrc, nWidth);
}

void Functions::prefetch_nothing_c(const Byte *pDst, int nWidth)
{
   UNUSED(pDst);
   UNUSED(nWidth);
}

extern "C" void start_asm(Byte *pbBytes);
extern "C" void stop_asm(const Byte *pbBytes);

Functions::Asm::Asm()
{
    pbBytes = new Byte[16*10];
    start_asm(pbBytes);
}

Functions::Asm::~Asm()
{
    stop_asm(pbBytes);
    delete[] pbBytes;
}

extern "C" unsigned int cpuid_asm( unsigned int nOp, unsigned int *pEax, unsigned int *pEbx, unsigned int *pEcx, unsigned int *pEdx);
extern "C" unsigned int cpu_test_asm( );

CpuFlags Functions::get_cpu_flags()
{
   unsigned int nEax, nEbx, nEcx, nEdx;
   unsigned int vendor[4] = { 0 };
   CpuFlags flags = CPU_NONE;

   if ( !cpu_test_asm() )
      return flags;

   cpuid_asm( 0, &nEax, vendor + 0, vendor + 2, vendor + 1 );

   if ( !nEax )
      return flags;

   cpuid_asm( 1, &nEax, &nEbx, &nEcx, &nEdx );

   /* default checks */
   if ( nEdx & 0x00800000 ) flags |= CPU_MMX;
   else return flags;
   if ( nEdx & 0x02000000 ) flags |= CPU_ISSE;
   if ( nEdx & 0x04000000 ) flags |= CPU_SSE2;
   if ( nEcx & 0x00000001 ) flags |= CPU_SSE3;
   if ( nEcx & 0x00000200 ) flags |= CPU_SSSE3;

   cpuid_asm( 0x80000000, &nEax, &nEbx, &nEcx, &nEdx );

   /* amd specific checks */
   if ( !strcmp((char*)vendor, "AuthenticAMD") && nEax >= 0x80000001 )
   {
      cpuid_asm( 0x80000001, &nEax, &nEbx, &nEcx, &nEdx );
      if ( nEdx & 0x80000000 ) flags |= CPU_3DNOW;
      if ( nEdx & 0x00400000 ) flags |= CPU_ISSE;
   }

   return flags;
} 
