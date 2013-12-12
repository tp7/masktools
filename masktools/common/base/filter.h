#ifndef __Mt_Base_H__
#define __Mt_Base_H__

#include "../../../common/functions/functions.h"
#include "../../../common/params/params.h"
#include "../../../common/parser/parser.h"
#include "../../../common/utils/utils.h"
#include "../../common/params/params.h"
#include "../../common/clip/inputconfig.h"

namespace Filtering { namespace MaskTools { 

enum class FilterProcessingType {
    INPLACE,
    CHILD
};

class Filter {

protected:
    ClipArray childs;
    Parameters parameters;

    int nWidth, nHeight;
    Colorspace C;

    String error;
    CpuFlags flags;
    Operator operators[3];

    bool inPlace_;
    int nXOffset, nYOffset, nXOffsetUV, nYOffsetUV;
    int nCoreWidth, nCoreHeight, nCoreWidthUV, nCoreHeightUV;

    std::vector<Frame<const Byte> > frames;
    Constraint constraints[3];

   virtual void process(int n, const Plane<Byte> &dst, int nPlane) = 0;
   virtual InputConfiguration &input_configuration() const = 0;

   static Signature &add_defaults(Signature &signature)
   {
      signature.add( Parameter( 3, "Y" ) );
      signature.add( Parameter( 1, "U" ) );
      signature.add( Parameter( 1, "V" ) );
      signature.add( Parameter( Value( String( "" ) ), "chroma" ) );
      signature.add( Parameter( 0, "offX" ) );
      signature.add( Parameter( 0, "offY" ) );
      signature.add( Parameter( -1, "w" ) );
      signature.add( Parameter( -1, "h" ) );
      signature.add( Parameter( true, "sse2" ) );
      signature.add( Parameter( true, "sse3" ) );
      signature.add( Parameter( true, "ssse3" ) );
      signature.add( Parameter( true, "sse4" ) );

      return signature;
   }

public:

    Filter(const Parameters &parameters, FilterProcessingType processingType) :
        parameters(parameters),
        flags(Functions::get_cpu_flags()),
        inPlace_(processingType == FilterProcessingType::INPLACE),
        nXOffset(parameters["offx"].toInt()),
        nYOffset(parameters["offy"].toInt()),
        nCoreWidth(parameters["w"].toInt()),
        nCoreHeight(parameters["h"].toInt())
    {
        for (auto &param: parameters) {
            if (param.getType() == TYPE_CLIP) {
                childs.push_back(param.getValue().toClip());
            }
        }

        assert(!childs.empty());

        nWidth = childs[0]->width();
        nHeight = childs[0]->height();
        C = childs[0]->colorspace();

        operators[0] = Operator(parameters["Y"].toInt());
        operators[1] = Operator(parameters["U"].toInt());
        operators[2] = Operator(parameters["V"].toInt());

        if (nXOffset < 0 || nXOffset > nWidth) nXOffset = 0;
        if (nYOffset < 0 || nYOffset > nHeight) nYOffset = 0;
        if (nXOffset + nCoreWidth  > nWidth  || nCoreWidth  < 0) nCoreWidth = nWidth - nXOffset;
        if (nYOffset + nCoreHeight > nHeight || nCoreHeight < 0) nCoreHeight = nHeight - nYOffset;

        if (parameters["chroma"].is_defined())
        {
            /* overrides chroma channel operators according to the "chroma" string */
            String chroma = parameters["chroma"].toString();

            if (chroma == "process")
                operators[1] = operators[2] = PROCESS;
            else if (chroma == "copy")
                operators[1] = operators[2] = COPY;
            else if (chroma == "copy first")
                operators[1] = operators[2] = COPY;
            else if (chroma == "copy second")
                operators[1] = operators[2] = COPY_SECOND;
            else if (chroma == "copy third")
                operators[1] = operators[2] = COPY_THIRD;
            else
                operators[1] = operators[2] = Operator(MEMSET, atoi(chroma.c_str()));
        }

        /* checks the operators */
        for (int i = 0; i < 3; i++)
        {
            if (operators[i] == COPY_THIRD && childs.size() < 3)
                operators[i] = COPY_SECOND;
            if (operators[i] == COPY_SECOND && childs.size() < 2)
                operators[i] = COPY;
        }

        if (is_in_place())
        {
            /* in place filters copy differently */
            for (int i = 0; i < 3; i++)
            {
                switch (operators[i].getMode())
                {
                case COPY: operators[i] = NONE; break;
                case COPY_SECOND: operators[i] = COPY; break;
                case COPY_THIRD: operators[i] = COPY_SECOND; break;
                }
            }
        }

        /* effective modes */
        print(LOG_DEBUG, "modes : %i %i %i\n", operators[0].getMode(), operators[1].getMode(), operators[2].getMode());

        /* cpu flags */
        if (!parameters["sse2"].toBool()) flags &= ~CPU_SSE2;
        if (!parameters["sse3"].toBool()) flags &= ~CPU_SSE3;
        if (!parameters["ssse3"].toBool()) flags &= ~CPU_SSSE3;
        if (!parameters["sse4"].toBool()) {
            flags &= ~CPU_SSE4_1;
            flags &= ~CPU_SSE4_2;
        }

        print(LOG_DEBUG, "using cpu flags : 0x%x\n", flags);

        /* chroma offsets and box */
        if (C != COLORSPACE_Y8 && C != COLORSPACE_NONE)
        {
            nXOffsetUV = nXOffset / width_ratios[1][C];
            nYOffsetUV = nYOffset / height_ratios[1][C];
            nCoreWidthUV = (nXOffset + nCoreWidth) / width_ratios[1][C] - nXOffsetUV;
            nCoreHeightUV = (nYOffset + nCoreHeight) / height_ratios[1][C] - nYOffsetUV;
        }

        /* effective offset */
        print(LOG_DEBUG, "offset : %i %i, width x height : %i x %i\n", nXOffset, nYOffset, nCoreWidth, nCoreHeight);

        /* check the colorspace */
        if (C == COLORSPACE_NONE)
            error = "unsupported colorspace. masktools only support planar YUV colorspaces (YV12, YV16, YV24)";
    }

    void process_plane(int n, const Plane<Byte> &output_plane, int nPlane)
    {
        /* need multiple constraints */
        Constraint constraint = constraints[nPlane];

        switch (operators[nPlane].getMode())
        {
        case COPY:
            Functions::copy_plane(output_plane.data(), output_plane.pitch(),
                frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
                output_plane.width(), output_plane.height());
            break;
        case COPY_SECOND:
            Functions::copy_plane(output_plane.data(), output_plane.pitch(),
                frames[1].plane(nPlane).data(), frames[1].plane(nPlane).pitch(),
                output_plane.width(), output_plane.height());
            break;
        case COPY_THIRD:
            Functions::copy_plane(output_plane.data(), output_plane.pitch(),
                frames[2].plane(nPlane).data(), frames[2].plane(nPlane).pitch(),
                output_plane.width(), output_plane.height());
            break;
        case MEMSET:
            Functions::memset_plane(output_plane.data(), output_plane.pitch(), 
                output_plane.width(), output_plane.height(), 
                static_cast<Byte>(operators[nPlane].value()));
            break;
        case PROCESS:
            process(n, output_plane, nPlane);
            break;
        case NONE:
        default: break;
        }
    }

    virtual Frame<Byte> get_frame(int n, const Frame<Byte> &output_frame)
    {
        Frame<Byte> output = output_frame.offset(nXOffset, nYOffset, nCoreWidth, nCoreHeight);

        for (int i = 0; i < int(input_configuration().size()); i++) {
            frames.push_back(childs[input_configuration()[i].index()]->get_const_frame(n + input_configuration()[i].offset())
                .offset(nXOffset, nYOffset, nCoreWidth, nCoreHeight));
        }

        for (int i = 0; i < plane_counts[C]; i++) {
            constraints[i] = Constraint(flags, output.plane(i));
        }

        for (int i = 0; i < int(frames.size()); i++) {
            for (int j = 0; j < plane_counts[frames[i].colorspace()]; j++) {
                constraints[j] = Constraint(constraints[j], frames[i].plane(j));
            }
        }

        for (int i = 0; i < plane_counts[C]; i++) {
            process_plane(n, output.plane(i), i);
        }

        frames.clear();

        return output_frame;
    }

    ClipArray &get_childs() { return childs; }

    String get_error() const { return error; }
    bool is_error() const { return !error.empty(); }
    bool is_in_place() const { return inPlace_; }

};

} } // namespace MaskTools, Filtering

#endif
