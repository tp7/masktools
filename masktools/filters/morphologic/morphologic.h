#ifndef __Mt_MorphologicFilter_H__
#define __Mt_MorphologicFilter_H__

#include "../../common/base/filter.h"
#include "../../../common/parser/parser.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic {

typedef void (Processor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nMaxDeviation, const int *pCoordinates, int nCoordinates, int nWidth, int nHeight);

class MorphologicFilter : public MaskTools::Filter
{
    int max_deviations[3];
    int *coorinates_list;
    int coordinates_count;

    MorphologicFilter(const MorphologicFilter &filter);

protected:

    ProcessorList<Processor> processors;

    virtual void process(int n, const Plane<Byte> &dst, int nPlane, const ::Filtering::Frame<const Byte> frames[3], const Constraint constraints[3]) override
    {
        UNUSED(n);
        processors.best_processor(constraints[nPlane])(dst.data(), dst.pitch(),
            frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
            max_deviations[nPlane], coorinates_list, coordinates_count, dst.width(), dst.height());
    }

    void FillCoordinates(const String &coordinates)
    {
        auto coeffs = Parser::getDefaultParser().parse(coordinates, " (),;.").getExpression();
        coordinates_count = coeffs.size();
        coorinates_list = new int[coordinates_count];
        int i = 0;

        while (!coeffs.empty())
        {
            coorinates_list[i++] = int(coeffs.front().getValue(0, 0, 0));
            coeffs.pop_front();
        }
    }

public:
    MorphologicFilter(const Parameters &parameters) : MaskTools::Filter(parameters, FilterProcessingType::CHILD), coorinates_list(NULL), coordinates_count(0)
    {
        max_deviations[0] = clip<int, int>(parameters["thY"].toInt(), 0, 255);
        max_deviations[1] = clip<int, int>(parameters["thC"].toInt(), 0, 255);
        max_deviations[2] = clip<int, int>(parameters["thC"].toInt(), 0, 255);
    }

    ~MorphologicFilter()
    {
        delete[] coorinates_list;
        coorinates_list = NULL;
    }

    InputConfiguration &input_configuration() const { return OneFrame(); }
};

} } } } // namespace Morphologic, Filters, MaskTools, Filtering

#endif