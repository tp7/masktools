#include <emmintrin.h>


static inline __m128i simd_load_epi128(const __m128i* ptr) {
    return _mm_load_si128(ptr);
}

static inline __m128i simd_loadu_epi128(const __m128i* ptr) {
    return _mm_loadu_si128(ptr);
}

static inline void simd_store_epi128(__m128i *ptr, __m128i value) {
    _mm_store_si128(ptr, value);
}

static inline void simd_storeu_epi128(__m128i *ptr, __m128i value) {
    _mm_storeu_si128(ptr, value);
}