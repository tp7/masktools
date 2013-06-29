#ifndef __Mt_SIMD_H__
#define __Mt_SIMD_H__

#include <emmintrin.h>

//because ICC is smart enough on its own and force inlining actually makes it slower
#ifdef __INTEL_COMPILER
#define FORCEINLINE inline
#else
#define FORCEINLINE __forceinline
#endif

#define USE_MOVPS

extern "C" {

static FORCEINLINE __m128i simd_load_epi128(const __m128i* ptr) {
#ifdef USE_MOVPS
    return _mm_castps_si128(_mm_load_ps(reinterpret_cast<const float*>(ptr)));
#else
    return _mm_load_si128(ptr);
#endif
}

static FORCEINLINE __m128i simd_loadu_epi128(const __m128i* ptr) {
#ifdef USE_MOVPS
    return _mm_castps_si128(_mm_loadu_ps(reinterpret_cast<const float*>(ptr)));
#else
    return _mm_loadu_si128(ptr);
#endif
}

static FORCEINLINE void simd_store_epi128(__m128i *ptr, __m128i value) {
#ifdef USE_MOVPS
    _mm_store_ps(reinterpret_cast<float*>(ptr), _mm_castsi128_ps(value));
#else
    _mm_store_si128(ptr, value);
#endif
}

static FORCEINLINE void simd_storeu_epi128(__m128i *ptr, __m128i value) {
#ifdef USE_MOVPS
    _mm_storeu_ps(reinterpret_cast<float*>(ptr), _mm_castsi128_ps(value));
#else
    _mm_storeu_si128(ptr, value);
#endif
}

// because for some reason _mm_set1_epi8 is super slow with vc110
static FORCEINLINE __m128i simd_set8_epi32(unsigned int value) {
    return _mm_set1_epi32((value << 24) | (value << 16) | (value << 8) | value);
}

};
#endif __Mt_SIMD_H__
