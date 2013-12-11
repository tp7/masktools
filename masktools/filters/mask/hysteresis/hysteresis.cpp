#include "hysteresis.h"

using namespace Filtering;


struct Coordinates {
    int x;
    int y;

    Coordinates(int x_, int y_) : x(x_), y(y_) {}
};

typedef std::vector<Coordinates> CoordinatesList;

static void expand_mask(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc2, ptrdiff_t nSrc2Pitch, Byte *pTemp, int x, int y, int nWidth, int nHeight, CoordinatesList &coordinates)
{
    //CoordinatesList coordinates;
    coordinates.clear();

    pTemp[0] = 255;
    pDst[0] = 255;

    coordinates.emplace_back(0, 0);

    while (!coordinates.empty())
    {
        /* pop last coordinates */
        Coordinates current = coordinates.back();
        coordinates.pop_back();

        /* check surrounding positions */
        int x_min = current.x  == -x ? current.x : current.x - 1;
        int x_max = current.x  == nWidth - x - 1 ? current.x + 1 : current.x + 2;
        int y_min = current.y == -y ? current.y : current.y - 1;
        int y_max = current.y == nHeight - y - 1 ? current.y + 1 : current.y + 2;

        for (int i = y_min; i < y_max; i++) {
            for (int j = x_min; j < x_max; j++) {
                if (!pTemp[j + i * nWidth] && pSrc2[j + i * nSrc2Pitch]) {
                    coordinates.emplace_back(j, i);
                    pTemp[j + i * nWidth] = 255;
                    pDst[j + i * nDstPitch] = 255;
                }
            }
        }
    }
}

void Filtering::MaskTools::Filters::Mask::Hysteresis::hysteresis_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch,
    const Byte *pSrc2, ptrdiff_t nSrc2Pitch, Byte *pTemp, int width, int height)
{

    memset(pDst, 0, nDstPitch * height * sizeof(Byte));
    memset(pTemp, 0, width   * height * sizeof(Byte));
    CoordinatesList coordinates;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)  {
            if (!pTemp[x] && pSrc1[x] && pSrc2[x]) {
                expand_mask(pDst + x, nDstPitch, pSrc2 + x, nSrc2Pitch, pTemp + x, x, y, width, height, coordinates);
            }
        }
        pTemp += width;
        pSrc1 += nSrc1Pitch;
        pSrc2 += nSrc2Pitch;
        pDst += nDstPitch;
    }
}