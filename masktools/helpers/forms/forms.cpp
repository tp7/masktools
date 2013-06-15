#include "forms.h"
#include <ostream>

namespace Filtering { namespace MaskTools { namespace Helpers { namespace Forms {

static String &append_vector(String &str, int nX, int nY)
{
   return str.append(std::to_string(nX)).append(" ").append(std::to_string(nY)).append(" ");
}

#define ZERO ( zero || !i || !j )

String RectangleToString(int nTopLeftX, int nTopLeftY, int nBottomRightX, int nBottomRightY, bool zero)
{
   String result = "";

   for ( int i = nTopLeftX; i <= nBottomRightX; i++ )
      for ( int j = nTopLeftY; j <= nBottomRightY; j++ )
         if ( ZERO )
            result = append_vector(result, i, j);
 
   return result;
}

String LosangeToString(int nTopLeftX, int nTopLeftY, int nBottomRightX, int nBottomRightY, bool zero)
{
   String result = "";

   int nVerticalDiameter = nBottomRightY - nTopLeftY;
   int nHorizontalDiameter = nBottomRightX - nTopLeftX;
   int nCenterX = nBottomRightX + nTopLeftX;
   int nCenterY = nBottomRightY + nTopLeftY;

   for ( int i = nTopLeftX; i <= nBottomRightX; i++ )
      for ( int j = nTopLeftY; j <= nBottomRightY; j++ )
         if ( abs( 2 * i - nCenterX ) * nVerticalDiameter + abs( j * 2 - nCenterY) * nHorizontalDiameter <= nHorizontalDiameter * nVerticalDiameter && ZERO )
            result = append_vector(result, i, j);
 
   return result;
}

String EllipseToString(int nTopLeftX, int nTopLeftY, int nBottomRightX, int nBottomRightY, bool zero)
{
   String result = "";

   int nVerticalDiameter = nBottomRightY - nTopLeftY;
   int nHorizontalDiameter = nBottomRightX - nTopLeftX;
   int nCenterX = nBottomRightX + nTopLeftX;
   int nCenterY = nBottomRightY + nTopLeftY;

   for ( int i = nTopLeftX; i <= nBottomRightX; i++ )
      for ( int j = nTopLeftY; j <= nBottomRightY; j++ )
         if ( ( 2 * i - nCenterX ) * ( 2 * i - nCenterX ) * nVerticalDiameter * nVerticalDiameter + ( 2 * j - nCenterY ) * ( 2 * j - nCenterY ) * nHorizontalDiameter * nHorizontalDiameter <= nHorizontalDiameter * nHorizontalDiameter * nVerticalDiameter * nVerticalDiameter && ZERO )
            result = append_vector(result, i, j);
 
   return result;
}

} } } }