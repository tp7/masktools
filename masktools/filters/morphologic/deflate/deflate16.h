#ifndef __Mt_Deflate16_H__
#define __Mt_Deflate16_H__

#include "../morphologic16.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic16 { namespace Deflate16 {

extern StackedProcessor *deflate_stacked_c;
extern InterleavedProcessor *deflate_interleaved_c;

class Deflate16 : public Morphologic16::MorphologicFilter16
{
public:
    Deflate16(const Parameters &parameters) : Morphologic16::MorphologicFilter16(parameters)
    {      
        if (parameters["stacked"].toBool()) {
            /* add the processors */
            stackedProcessors.push_back(Filtering::Processor<StackedProcessor>(deflate_stacked_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
        } else {
            interleavedProcessors.push_back(Filtering::Processor<InterleavedProcessor>(deflate_interleaved_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
        }
    }

    static Signature Deflate16::filter_signature()
    {
        Signature signature = "mt_deflate16";

        signature.add( Parameter(TYPE_CLIP, "") );
        signature.add( Parameter(65535, "thY") );
        signature.add( Parameter(65535, "thC") );
        signature.add( Parameter( false, "stacked" ) );

        return add_defaults( signature );
    }
};

} } } } } // namespace Deflate, Morphologic, Filter, MaskTools, Filtering

#endif