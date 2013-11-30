#include "logic.h"
#include "../../common/simd.h"

using namespace Filtering;

inline Byte add(Byte a, Byte b) { return clip<Byte, int>(a + (int)b); }
inline Byte sub(Byte a, Byte b) { return clip<Byte, int>(a - (int)b); }
inline Byte nop(Byte a, Byte b) { UNUSED(b); return a; }

inline Byte and(Byte a, Byte b, Byte th1, Byte th2) { UNUSED(th1); UNUSED(th2); return a & b; }
inline Byte or(Byte a, Byte b, Byte th1, Byte th2) { UNUSED(th1); UNUSED(th2); return a | b; }
inline Byte andn(Byte a, Byte b, Byte th1, Byte th2) { UNUSED(th1); UNUSED(th2); return a & ~b; }
inline Byte xor(Byte a, Byte b, Byte th1, Byte th2) { UNUSED(th1); UNUSED(th2); return a ^ b; }

template <decltype(add) opa, decltype(add) opb>
inline Byte min_t(Byte a, Byte b, Byte th1, Byte th2) { 
    return min<Byte>(opa(a, th1), opb(b, th2)); 
}

template <decltype(add) opa, decltype(add) opb>
inline Byte max_t(Byte a, Byte b, Byte th1, Byte th2) { 
    return max<Byte>(opa(a, th1), opb(b, th2)); 
}

template <decltype(and) op>
void logic_t(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight, Byte nThresholdDestination, Byte nThresholdSource)
{
   for ( int y = 0; y < nHeight; y++ )
   {
      for ( int x = 0; x < nWidth; x++ )
         pDst[x] = op(pDst[x], pSrc[x], nThresholdDestination, nThresholdSource);
      pDst += nDstPitch;
      pSrc += nSrcPitch;
   }
}

/* sse2 */

static inline __m128i add_sse2(__m128i a, __m128i b) { return _mm_adds_epu8(a, b); }
static inline __m128i sub_sse2(__m128i a, __m128i b) { return _mm_subs_epu8(a, b); }
static inline __m128i nop_sse2(__m128i a, __m128i) { return a; }

static inline __m128i and_sse2_op(const __m128i &a, const __m128i &b, const __m128i&, const __m128i&) { 
    return _mm_and_si128(a, b); 
}

static inline __m128i or_sse2_op(const __m128i &a, const __m128i &b, const __m128i&, const __m128i&) { 
    return _mm_or_si128(a, b); 
}

static inline __m128i andn_sse2_op(const __m128i &a, const __m128i &b, const __m128i&, const __m128i&) { 
    return _mm_andnot_si128(a, b); 
}

static inline __m128i xor_sse2_op(const __m128i &a, const __m128i &b, const __m128i&, const __m128i&) { 
    return _mm_xor_si128(a, b); 
}

template <decltype(add_sse2) opa, decltype(add_sse2) opb>
static inline __m128i min_t_sse2(const __m128i &a, const __m128i &b, const __m128i& th1, const __m128i& th2) { 
    return _mm_min_epu8(opa(a, th1), opb(b, th2));
}

template <decltype(add_sse2) opa, decltype(add_sse2) opb>
static inline __m128i max_t_sse2(const __m128i &a, const __m128i &b, const __m128i& th1, const __m128i& th2) { 
    return _mm_max_epu8(opa(a, th1), opb(b, th2));
}


template<MemoryMode mem_mode, decltype(and_sse2_op) op, decltype(and) op_c>
    void logic_t_sse2(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight, Byte nThresholdDestination, Byte nThresholdSource)
{
    int wMod32 = (nWidth / 32) * 32;
    auto pDst2 = pDst;
    auto pSrc2 = pSrc;
    auto tDest = _mm_set1_epi8(Byte(nThresholdDestination));
    auto tSource = _mm_set1_epi8(Byte(nThresholdSource));

    for ( int j = 0; j < nHeight; ++j ) {
        for ( int i = 0; i < wMod32; i+=32 ) {
            _mm_prefetch(reinterpret_cast<const char*>(pDst)+i+384, _MM_HINT_T0);
            _mm_prefetch(reinterpret_cast<const char*>(pSrc)+i+384, _MM_HINT_T0);

            auto dst = simd_load_epi128<mem_mode>(pDst+i);
            auto dst2 = simd_load_epi128<mem_mode>(pDst+i+16);
            auto src = simd_load_epi128<mem_mode>(pSrc+i);
            auto src2 = simd_load_epi128<mem_mode>(pSrc+i+16);

            auto result = op(dst, src, tDest, tSource);
            auto result2 = op(dst2, src2, tDest, tSource);

            simd_store_epi128<mem_mode>(pDst+i, result);
            simd_store_epi128<mem_mode>(pDst+i+16, result2);
        }
        pDst += nDstPitch;
        pSrc += nSrcPitch;
    }

    if (nWidth > wMod32) {
        logic_t<op_c>(pDst2 + wMod32, nDstPitch, pSrc2 + wMod32, nSrcPitch, nWidth - wMod32, nHeight, nThresholdDestination, nThresholdSource);
    }
}



namespace Filtering { namespace MaskTools { namespace Filters { namespace Logic {

Processor *and_c  = &logic_t<and>;
Processor *or_c   = &logic_t<or>;
Processor *andn_c = &logic_t<andn>;
Processor *xor_c  = &logic_t<xor>;

#define DEFINE_C_VERSIONS(mode) \
    Processor *mode##_c         = &logic_t<mode##_t<nop, nop>>;   \
    Processor *mode##sub_c      = &logic_t<mode##_t<nop, sub>>;   \
    Processor *mode##add_c      = &logic_t<mode##_t<nop, add>>;   \
    Processor *sub##mode##_c    = &logic_t<mode##_t<sub, nop>>;   \
    Processor *sub##mode##sub_c = &logic_t<mode##_t<sub, sub>>;   \
    Processor *sub##mode##add_c = &logic_t<mode##_t<sub, add>>;   \
    Processor *add##mode##_c    = &logic_t<mode##_t<add, nop>>;   \
    Processor *add##mode##sub_c = &logic_t<mode##_t<add, sub>>;   \
    Processor *add##mode##add_c = &logic_t<mode##_t<add, add>>;

DEFINE_C_VERSIONS(min);
DEFINE_C_VERSIONS(max);


#define DEFINE_SSE2_VERSIONS(name, mem_mode) \
Processor *and_##name  = &logic_t_sse2<mem_mode, and_sse2_op, and>; \
Processor *or_##name   = &logic_t_sse2<mem_mode, or_sse2_op, or>; \
Processor *andn_##name = &logic_t_sse2<mem_mode, andn_sse2_op, andn>; \
Processor *xor_##name  = &logic_t_sse2<mem_mode, xor_sse2_op, xor>;

DEFINE_SSE2_VERSIONS(sse2, MemoryMode::SSE2_UNALIGNED)
DEFINE_SSE2_VERSIONS(asse2, MemoryMode::SSE2_ALIGNED)

#define DEFINE_SILLY_SSE2_VERSIONS(mode, name, mem_mode) \
Processor *mode##_##name         = &logic_t_sse2<mem_mode, mode##_t_sse2<nop_sse2, nop_sse2>, mode##_t<nop, nop>>;   \
Processor *mode##sub_##name      = &logic_t_sse2<mem_mode, mode##_t_sse2<nop_sse2, sub_sse2>, mode##_t<nop, sub>>;   \
Processor *mode##add_##name      = &logic_t_sse2<mem_mode, mode##_t_sse2<nop_sse2, add_sse2>, mode##_t<nop, add>>;   \
Processor *sub##mode##_##name    = &logic_t_sse2<mem_mode, mode##_t_sse2<sub_sse2, nop_sse2>, mode##_t<sub, nop>>;   \
Processor *sub##mode##sub_##name = &logic_t_sse2<mem_mode, mode##_t_sse2<sub_sse2, sub_sse2>, mode##_t<sub, sub>>;   \
Processor *sub##mode##add_##name = &logic_t_sse2<mem_mode, mode##_t_sse2<sub_sse2, add_sse2>, mode##_t<sub, add>>;   \
Processor *add##mode##_##name    = &logic_t_sse2<mem_mode, mode##_t_sse2<add_sse2, nop_sse2>, mode##_t<add, nop>>;   \
Processor *add##mode##sub_##name = &logic_t_sse2<mem_mode, mode##_t_sse2<add_sse2, sub_sse2>, mode##_t<add, sub>>;   \
Processor *add##mode##add_##name = &logic_t_sse2<mem_mode, mode##_t_sse2<add_sse2, add_sse2>, mode##_t<add, add>>;

DEFINE_SILLY_SSE2_VERSIONS(min, sse2, MemoryMode::SSE2_UNALIGNED)
DEFINE_SILLY_SSE2_VERSIONS(max, sse2, MemoryMode::SSE2_UNALIGNED)
DEFINE_SILLY_SSE2_VERSIONS(min, asse2, MemoryMode::SSE2_ALIGNED)
DEFINE_SILLY_SSE2_VERSIONS(max, asse2, MemoryMode::SSE2_ALIGNED)

} } } }