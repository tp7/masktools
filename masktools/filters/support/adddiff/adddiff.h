#ifndef __Mt_AddDiff_H__
#define __Mt_AddDiff_H__

#include "../../../common/base/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Support { namespace AddDiff {

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight);

Processor adddiff_c;
extern Processor *adddiff_sse2;
extern Processor *adddiff_asse2;

class AddDiff : public MaskTools::Filter
{
    ProcessorList<Processor> processors;

protected:
    virtual void process(int n, const Plane<Byte> &dst, int nPlane, const Filtering::Frame<const Byte> frames[3], const Constraint constraints[3]) override
    {
        UNUSED(n);
        processors.best_processor(constraints[nPlane])(dst.data(), dst.pitch(),
            frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
            dst.width(), dst.height());
    }

public:
    AddDiff(const Parameters &parameters) : MaskTools::Filter(parameters, FilterProcessingType::INPLACE) 
    {
        /* add the processors */
        processors.push_back(Filtering::Processor<Processor>(&adddiff_c, Constraint(CPU_NONE, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 1), 0));
        processors.push_back(Filtering::Processor<Processor>(adddiff_sse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 1), 1));
        processors.push_back(Filtering::Processor<Processor>(adddiff_asse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_16, 16), 2));
    }

    InputConfiguration &input_configuration() const { return InPlaceTwoFrame(); }

    static Signature filter_signature()
    {
        Signature signature = "mt_adddiff";

        signature.add(Parameter(TYPE_CLIP, ""));
        signature.add(Parameter(TYPE_CLIP, ""));

        return add_defaults(signature);
    }

};

} } } } }

#endif