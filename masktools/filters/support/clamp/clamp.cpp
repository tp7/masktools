#include "clamp.h"
#include "../../../common/simd.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Support  { namespace Clamp {

void clamp_c(Byte *pDst, ptrdiff_t dst_pitch, const Byte *pSrc1, ptrdiff_t src1_pitch, const Byte *pSrc2, ptrdiff_t src2_pitch, int width, int height, int overshoot, int undershoot)
{
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            pDst[x] = static_cast<Byte>(pDst[x] > pSrc1[x] + overshoot ? pSrc1[x] + overshoot : pDst[x]);
            pDst[x] = static_cast<Byte>(pDst[x] < pSrc2[x] - undershoot ? pSrc2[x] - undershoot : pDst[x]);
        }
        pDst += dst_pitch;
        pSrc1 += src1_pitch;
        pSrc2 += src2_pitch;
    }
}

template<MemoryMode mem_mode>
static void clamp_sse2_t(Byte *pDst, ptrdiff_t dst_pitch, const Byte *pSrc1, ptrdiff_t src1_pitch, const Byte *pSrc2, ptrdiff_t src2_pitch, int width, int height, int overshoot, int undershoot)
{
    int mod16_width = (width / 16) * 16;
    auto pDst_s = pDst;
    auto pSrc1_s = pSrc1;
    auto pSrc2_s = pSrc2;

    auto overshoot_v = _mm_set1_epi8(Byte(overshoot));
    auto undershoot_v = _mm_set1_epi8(Byte(undershoot));

    for ( int j = 0; j < height; ++j ) {
        for ( int i = 0; i < mod16_width; i+=16 ) {
            auto upper_limit = simd_load_si128<mem_mode>(pSrc1+i);
            auto lower_limit = simd_load_si128<mem_mode>(pSrc2+i);

            upper_limit = _mm_adds_epu8(upper_limit, overshoot_v);
            lower_limit = _mm_subs_epu8(lower_limit, undershoot_v);

            auto limited = simd_load_si128<mem_mode>(pDst+i);

            limited = _mm_min_epu8(limited, upper_limit);
            limited = _mm_max_epu8(limited, lower_limit);

            simd_store_si128<mem_mode>(pDst+i, limited);
        }
        pDst += dst_pitch;
        pSrc1 += src1_pitch;
        pSrc2 += src2_pitch;
    }

    if (width > mod16_width) {
        clamp_c(pDst_s + mod16_width, dst_pitch, pSrc1_s + mod16_width, src1_pitch, pSrc2_s+mod16_width, src2_pitch, width - mod16_width, height, overshoot, undershoot);
    }
}

Processor *clamp_sse2 = &clamp_sse2_t<MemoryMode::SSE2_UNALIGNED>;
Processor *clamp_asse2 = &clamp_sse2_t<MemoryMode::SSE2_ALIGNED>;

} } } } }