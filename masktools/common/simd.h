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

enum class MemoryMode {
    SSE2_UNALIGNED,
    SSE2_ALIGNED
};


template<MemoryMode mem_mode, typename T>
static MT_FORCEINLINE __m128i simd_load_si128(const T* ptr) {
#ifdef USE_MOVPS
    if (mem_mode == MemoryMode::SSE2_ALIGNED) {
        return _mm_castps_si128(_mm_load_ps(reinterpret_cast<const float*>(ptr)));
    } else {
        return _mm_castps_si128(_mm_loadu_ps(reinterpret_cast<const float*>(ptr)));
    }
#else
    if (mem_mode == MemoryMode::SSE2_ALIGNED) {
        return _mm_load_si128(reinterpret_cast<__m128i*>(ptr));
    } else {
        return _mm_loadu_si128(reinterpret_cast<__m128i*>(ptr));
    }
#endif
}


template<MemoryMode mem_mode, typename T>
static MT_FORCEINLINE void simd_store_si128(T *ptr, __m128i value) {
#ifdef USE_MOVPS
    if (mem_mode == MemoryMode::SSE2_ALIGNED) {
        _mm_store_ps(reinterpret_cast<float*>(ptr), _mm_castsi128_ps(value));
    } else {
        _mm_storeu_ps(reinterpret_cast<float*>(ptr), _mm_castsi128_ps(value));
    }
#else
    if (mem_mode == MemoryMode::SSE2_ALIGNED) {
        _mm_store_si128(reinterpret_cast<__m128i*>(ptr), value);
    } else {   
        _mm_storeu_si128(reinterpret_cast<__m128i*>(ptr), value);
    }
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



enum class Border {
    Left,
    Right,
    None
};

#pragma warning(disable: 4309)

template<Border border_mode, MemoryMode mem_mode>
static MT_FORCEINLINE __m128i load_one_to_left(const Byte *ptr) {
    if (border_mode == Border::Left) {
        auto mask_left = _mm_setr_epi8(0xFF, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00);
        auto val = simd_load_si128<mem_mode>(reinterpret_cast<const __m128i*>(ptr));
        return _mm_or_si128(_mm_slli_si128(val, 1), _mm_and_si128(val, mask_left));
    } else {
        return simd_load_si128<MemoryMode::SSE2_UNALIGNED>(reinterpret_cast<const __m128i*>(ptr - 1));
    }
}

template<Border border_mode, MemoryMode mem_mode>
static MT_FORCEINLINE __m128i load_one_to_right(const Byte *ptr) {
    if (border_mode == Border::Right) {
        auto mask_right = _mm_setr_epi8(00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 0xFF);
        auto val = simd_load_si128<mem_mode>(reinterpret_cast<const __m128i*>(ptr));
        return _mm_or_si128(_mm_srli_si128(val, 1), _mm_and_si128(val, mask_right));
    } else {
        return simd_load_si128<MemoryMode::SSE2_UNALIGNED>(reinterpret_cast<const __m128i*>(ptr + 1));
    }
}

static MT_FORCEINLINE __m128i threshold_sse2(const __m128i &value, const __m128i &lowThresh, const __m128i &highThresh, const __m128i &v128) {
    auto sat = _mm_sub_epi8(value, v128);
    auto low = _mm_cmpgt_epi8(sat, lowThresh);
    auto high = _mm_cmpgt_epi8(sat, highThresh);
    auto result = _mm_and_si128(value, low);
    return _mm_or_si128(result, high);
}


#pragma warning(default: 4309)

#endif __Mt_SIMD_H__
