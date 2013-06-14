#include "filter.h"
//#include "../../common/parser/parser.h"
//
//using namespace MaskTools;
//using namespace MaskTools::Picture;
//using namespace MaskTools::Parameters;
//using namespace MaskTools::Constraints;
//using namespace MaskTools::Parser;
//
//namespace MaskTools { namespace Filters { namespace Morphologic {
//
//
//Filter::Filter(const Parameters::Parameters &parameters) : MaskTools::Filter(parameters), pCoordinates(NULL), nCoordinates(0)
//{
//   nMaxDeviation = clip(parameters["thY"].toInt(), 0, 255);
//   nMaxDeviationUV = clip(parameters["thC"].toInt(), 0, 255);
//}
//
//Filter::~Filter()
//{
//   if ( pCoordinates )
//      delete[] pCoordinates;
//   pCoordinates = NULL;
//}
//
//Clip::Inputs Filter::inputClips = Clip::OneFrame();
//
//} } }