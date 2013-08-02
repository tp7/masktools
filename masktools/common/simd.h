#ifndef __Mt_SIMD_H__
#define __Mt_SIMD_H__

#include <emmintrin.h>
#include "../../common/utils/utils.h"

using namespace Filtering;

//because ICC is smart enough on its own and force inlining actually makes it slower
#ifdef __INTEL_COMPILER
#define MT_FORCEINLINE inline
#else
#define MT_FORCEINLINE __forceinline
#endif

#define USE_MOVPS

extern "C" {

static MT_FORCEINLINE __m128i simd_load_epi128(const __m128i* ptr) {
#ifdef USE_MOVPS
    return _mm_castps_si128(_mm_load_ps(reinterpret_cast<const float*>(ptr)));
#else
    return _mm_load_si128(ptr);
#endif
}

static MT_FORCEINLINE __m128i simd_loadu_epi128(const __m128i* ptr) {
#ifdef USE_MOVPS
    return _mm_castps_si128(_mm_loadu_ps(reinterpret_cast<const float*>(ptr)));
#else
    return _mm_loadu_si128(ptr);
#endif
}

static MT_FORCEINLINE void simd_store_epi128(__m128i *ptr, __m128i value) {
#ifdef USE_MOVPS
    _mm_store_ps(reinterpret_cast<float*>(ptr), _mm_castsi128_ps(value));
#else
    _mm_store_si128(ptr, value);
#endif
}

static MT_FORCEINLINE void simd_storeu_epi128(__m128i *ptr, __m128i value) {
#ifdef USE_MOVPS
    _mm_storeu_ps(reinterpret_cast<float*>(ptr), _mm_castsi128_ps(value));
#else
    _mm_storeu_si128(ptr, value);
#endif
}

static MT_FORCEINLINE int simd_bit_scan_forward(int value) {
#ifdef __INTEL_COMPILER
    return _bit_scan_forward(value);
#else
    unsigned long index;
    _BitScanForward(&index, value);
    return index;
#endif
}

};

enum class Border {
    Left,
    Right,
    None
};

#pragma warning(disable: 4309)

template<bool isBorder, decltype(simd_load_epi128) load>
static MT_FORCEINLINE __m128i load_one_to_left(const Byte *ptr) {
    if (isBorder) {
        auto mask_left = _mm_setr_epi8(0xFF, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00);
        auto val = load(reinterpret_cast<const __m128i*>(ptr));
        return _mm_or_si128(_mm_slli_si128(val, 1), _mm_and_si128(val, mask_left));
    } else {
        return simd_loadu_epi128(reinterpret_cast<const __m128i*>(ptr - 1));
    }
}

template<bool isBorder, decltype(simd_load_epi128) load>
static MT_FORCEINLINE __m128i load_one_to_right(const Byte *ptr) {
    if (isBorder) {
        auto mask_right = _mm_setr_epi8(00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 0xFF);
        auto val = load(reinterpret_cast<const __m128i*>(ptr));
        return _mm_or_si128(_mm_srli_si128(val, 1), _mm_and_si128(val, mask_right));
    } else {
        return simd_loadu_epi128(reinterpret_cast<const __m128i*>(ptr + 1));
    }
}

#pragma warning(default: 4309)

static MT_FORCEINLINE __m128i simd_movehl_si128(const __m128i &a, const __m128i &b) {
    return _mm_castps_si128(_mm_movehl_ps(_mm_castsi128_ps(a), _mm_castsi128_ps(b)));
}

static MT_FORCEINLINE __m128i threshold_sse2(const __m128i &value, const __m128i &lowThresh, const __m128i &highThresh, const __m128i &v128) {
    auto sat = _mm_sub_epi8(value, v128);
    auto low = _mm_cmpgt_epi8(sat, lowThresh);
    auto high = _mm_cmpgt_epi8(sat, highThresh);
    auto result = _mm_and_si128(value, low);
    return _mm_or_si128(result, high);
}

template<CpuFlags flags>
static MT_FORCEINLINE __m128i simd_blend_epi8(__m128i const &selector, __m128i const &a, __m128i const &b) {
    if (flags >= CPU_SSE4_1) {
        return _mm_blendv_epi8 (b, a, selector);
    } else {
        return _mm_or_si128(_mm_and_si128(selector, a), _mm_andnot_si128(selector, b));
    }
}

template<CpuFlags flags>
static MT_FORCEINLINE __m128i simd_mullo_epi32(__m128i &a, __m128i &b) {
    if (flags >= CPU_SSE4_1) {
        return _mm_mullo_epi32(a, b);
    } else {
        auto a13    = _mm_shuffle_epi32(a, 0xF5);          // (-,a3,-,a1)
        auto b13    = _mm_shuffle_epi32(b, 0xF5);          // (-,b3,-,b1)
        auto prod02 = _mm_mul_epu32(a, b);                 // (-,a2*b2,-,a0*b0)
        auto prod13 = _mm_mul_epu32(a13, b13);             // (-,a3*b3,-,a1*b1)
        auto prod01 = _mm_unpacklo_epi32(prod02,prod13);   // (-,-,a1*b1,a0*b0) 
        auto prod23 = _mm_unpackhi_epi32(prod02,prod13);   // (-,-,a3*b3,a2*b2) 
        return _mm_unpacklo_epi64(prod01,prod23);   // (ab3,ab2,ab1,ab0)
    }
}

#endif __Mt_SIMD_H__
