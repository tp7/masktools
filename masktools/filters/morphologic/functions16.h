#ifndef __Mt_MorphologicFunctions16_H__
#define __Mt_MorphologicFunctions16_H__

#include "../../../common/utils/utils.h"
#include "../../common/simd.h"
#include "../../common/16bit.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic16 {

typedef Word (Operator)(Word, Word, Word, Word, Word, Word, Word, Word, Word, int);

enum Directions {
    Vertical = 1,
    Horizontal = 2,
    Both = Vertical | Horizontal,
    Square = 7
};

//height should be already divised by 2
template<Border borderMode, Operator op>
void process_line_morpho_stacked_c(Byte *pDst, const Byte *pSrcp, const Byte *pSrc, const Byte *pSrcn, int maxDeviation, int width, int height, int srcPitch, int dstPitch) {
    Byte *pDstLsb = pDst + height * dstPitch;
    const Byte *pSrcpLsb = pSrcp + height * srcPitch;
    const Byte *pSrcLsb = pSrc + height * srcPitch;
    const Byte *pSrcnLsb = pSrcn + height * srcPitch;
    const int leftOffset = borderMode == Border::Left ? 0 : 1;
    const int rightOffset = borderMode == Border::Right ? 0 : 1;

    for (int x = 0; x < width; ++x) {
        Word output = op(read_word_stacked(pSrcp, pSrcpLsb, x-leftOffset), 
            read_word_stacked(pSrcp, pSrcpLsb, x), 
            read_word_stacked(pSrcp, pSrcpLsb, x+rightOffset), 
            read_word_stacked(pSrc, pSrcLsb, x-leftOffset), 
            read_word_stacked(pSrc, pSrcLsb, x), 
            read_word_stacked(pSrc, pSrcLsb, x+rightOffset), 
            read_word_stacked(pSrcn, pSrcnLsb, x-leftOffset), 
            read_word_stacked(pSrcn, pSrcnLsb, x), 
            read_word_stacked(pSrcn, pSrcnLsb, x+rightOffset), 
            maxDeviation);
        
        write_word_stacked(pDst, pDstLsb, x, output);
    }
}

template<Border borderMode, Operator op>
void process_line_morpho_interleaved_c(Word *pDst, const Word *pSrcp, const Word *pSrc, const Word *pSrcn, int maxDeviation, int width, int height, int srcPitch, int dstPitch) {
    UNUSED(dstPitch); UNUSED(srcPitch); UNUSED(height);

    const int leftOffset = borderMode == Border::Left ? 0 : 1;
    const int rightOffset = borderMode == Border::Right ? 0 : 1;

    for (int x = 0; x < width; ++x) {
        pDst[x] = op(
            reinterpret_cast<const Word*>(pSrcp)[x-leftOffset], 
            reinterpret_cast<const Word*>(pSrcp)[x], 
            reinterpret_cast<const Word*>(pSrcp)[x+rightOffset], 
            reinterpret_cast<const Word*>(pSrc)[x-leftOffset], 
            reinterpret_cast<const Word*>(pSrc)[x], 
            reinterpret_cast<const Word*>(pSrc)[x+rightOffset], 
            reinterpret_cast<const Word*>(pSrcn)[x-leftOffset], 
            reinterpret_cast<const Word*>(pSrcn)[x], 
            reinterpret_cast<const Word*>(pSrcn)[x+rightOffset], 
            maxDeviation);
    }
}

template<class T>
struct MorphologicProcessor {
    typedef void (ProcessLineC)(T *pDst, const T *pSrcp, const T *pSrc, const T *pSrcn, int maxDeviation, int width, int height, int srcPitch, int dstPitch);

    template<ProcessLineC process_line_left, ProcessLineC process_line, ProcessLineC process_line_right>
    static void generic_c(T *pDst, ptrdiff_t nDstPitch, const T *pSrc, ptrdiff_t nSrcPitch, int nMaxDeviation, const int *pCoordinates, int nCoordinates, int nWidth, int nHeight)
    {
        const T *pSrcp = pSrc - nSrcPitch;
        const T *pSrcn = pSrc + nSrcPitch;


        UNUSED(nCoordinates); UNUSED(pCoordinates);

        /* top-left */
        process_line_left(pDst, pSrc, pSrc, pSrcn, nMaxDeviation, 1, nHeight, nSrcPitch, nDstPitch);

        /* top */
        process_line(pDst + 1, pSrc+1, pSrc+1, pSrcn+1, nMaxDeviation, nWidth - 2, nHeight, nSrcPitch, nDstPitch);

        /* top-right */
        process_line_right(pDst + nWidth - 1, pSrc + nWidth - 1, pSrc + nWidth - 1, pSrcn + nWidth - 1, nMaxDeviation, 1, nHeight, nSrcPitch, nDstPitch);

        pDst  += nDstPitch;
        pSrcp += nSrcPitch;
        pSrc  += nSrcPitch;
        pSrcn += nSrcPitch;

        for ( int y = 1; y < nHeight - 1; y++ )
        {
            /* left */
            process_line_left(pDst, pSrcp, pSrc, pSrcn, nMaxDeviation, 1, nHeight, nSrcPitch, nDstPitch);
            /* center */
            process_line(pDst + 1, pSrcp+1, pSrc+1, pSrcn+1, nMaxDeviation, nWidth - 2, nHeight, nSrcPitch, nDstPitch);
            /* right */
            process_line_right(pDst + nWidth - 1, pSrcp + nWidth - 1, pSrc + nWidth - 1, pSrcn + nWidth - 1, nMaxDeviation, 1, nHeight, nSrcPitch, nDstPitch);

            pDst  += nDstPitch;
            pSrcp += nSrcPitch;
            pSrc  += nSrcPitch;
            pSrcn += nSrcPitch;
        }

        /* bottom-left */
        process_line_left(pDst, pSrcp, pSrc, pSrc, nMaxDeviation, 1, nHeight, nSrcPitch, nDstPitch);
        /* bottom */
        process_line(pDst + 1, pSrcp+1, pSrc+1, pSrc+1, nMaxDeviation, nWidth - 2, nHeight, nSrcPitch, nDstPitch);
        /* bottom-right */
        process_line_right(pDst + nWidth - 1, pSrcp + nWidth - 1, pSrc + nWidth - 1, pSrc + nWidth - 1, nMaxDeviation, 1, nHeight, nSrcPitch, nDstPitch);
    }
};




template<class T>
void generic_custom_stacked_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nMaxDeviation, const int *pCoordinates, int nCoordinates, int nWidth, int nHeight)
{
    Byte *pDstLsb = pDst + nHeight * nDstPitch;
    const Byte *pSrcLsb = pSrc + nHeight * nDstPitch;

    for ( int j = 0; j < nHeight; j++ )
    {
        for ( int i = 0; i < nWidth; i++ )
        {
            T new_value(read_word_stacked(pSrc, pSrcLsb, i), nMaxDeviation);
            for ( int k = 0; k < nCoordinates; k+=2 )
            {
                if ( pCoordinates[k] + i >= 0 && pCoordinates[k] + i < nWidth &&
                    pCoordinates[k+1] + j >= 0 && pCoordinates[k+1] + j < nHeight ) {
                        new_value.add(read_word_stacked(pSrc, pSrcLsb, i + pCoordinates[k] + pCoordinates[k+1] * nSrcPitch));
                }
            }
            write_word_stacked(pDst, pDstLsb, i, new_value.finalize());
        }
        pSrc += nSrcPitch;
        pSrcLsb += nSrcPitch;
        pDst += nDstPitch;
        pDstLsb += nDstPitch;
    }
}


template<class T>
void generic_custom_interleaved_c(Word *pDst, ptrdiff_t nDstPitch, const Word *pSrc, ptrdiff_t nSrcPitch, int nMaxDeviation, const int *pCoordinates, int nCoordinates, int nWidth, int nHeight)
{
    for ( int j = 0; j < nHeight; j++ )
    {
        for ( int i = 0; i < nWidth; i++ )
        {
            T new_value(pSrc[i], nMaxDeviation);
            for ( int k = 0; k < nCoordinates; k+=2 )
            {
                if ( pCoordinates[k] + i >= 0 && pCoordinates[k] + i < nWidth &&
                    pCoordinates[k+1] + j >= 0 && pCoordinates[k+1] + j < nHeight ) {
                        new_value.add(pSrc[i + pCoordinates[k] + pCoordinates[k+1] * nSrcPitch]);
                }
            }
            pDst[i] = new_value.finalize();
        }
        pSrc += nSrcPitch;
        pDst += nDstPitch;
    }
}


} } } } // namespace Morphologic, Filters, MaskTools, Filtering

#endif