#ifndef __Mt_Hysteresis_H__
#define __Mt_Hysteresis_H__

#include "../../../common/base/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Mask { namespace Hysteresis {

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
                        const Byte *pSrc2, ptrdiff_t nSrc2Pitch, Byte *pTemp, int nWidth, int nHeight);

Processor hysteresis_c;

class Hysteresis : public MaskTools::Filter
{
    Byte *stack;

protected:
    virtual void process(int n, const Plane<Byte> &dst, int nPlane)
    {
        UNUSED(n);
        hysteresis_c(dst.data(), dst.pitch(),
            frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
            frames[1].plane(nPlane).data(), frames[1].plane(nPlane).pitch(),
            stack, dst.width(), dst.height());
    }

public:
    Hysteresis(const Parameters &parameters) : MaskTools::Filter(parameters, FilterProcessingType::CHILD), stack(nullptr) {
        stack = reinterpret_cast<Byte*>(_aligned_malloc(nWidth*nHeight, 16));
    }

    ~Hysteresis() {
        _aligned_free(stack);
    }

    InputConfiguration &input_configuration() const { return TwoFrame(); }

    static Signature filter_signature()
    {
        Signature signature = "mt_hysteresis";

        signature.add(Parameter(TYPE_CLIP, ""));
        signature.add(Parameter(TYPE_CLIP, ""));

        return add_defaults(signature);
    }
};


} } } } }

#endif