#ifndef __Mt_Params_H__
#define __Mt_Params_H__

#include "../../common/common.h"

namespace Filtering { namespace MaskTools {

typedef enum {
   NONE,
   MEMSET,
   COPY,
   PROCESS,
   COPY_SECOND,
   COPY_THIRD,

} Mode;

class Operator {

   Mode mode;
   int nValue;      /* only for mode == memset */
   static Operator IntToOperator(int nValue)
   {
      switch ( nValue )
      {
      case 5 : return Operator(COPY_THIRD);
      case 4 : return Operator(COPY_SECOND);
      case 3 : return Operator(PROCESS);
      case 2 : return Operator(COPY);
      case 1 : return Operator(NONE);
      default: return Operator(MEMSET, -nValue);
      }
   }

public:

   Operator() : mode(NONE), nValue(-1) {}
   Operator(Mode mode, int nValue = -1) : mode(mode), nValue(mode == MEMSET ? nValue : -1) {}
   Operator(int nValue)
   {
      this->nValue = -1;
      switch ( nValue )
      {
      case 5 : mode = COPY_THIRD; break;
      case 4 : mode = COPY_SECOND; break;
      case 3 : mode = PROCESS; break;
      case 2 : mode = COPY; break;
      case 1 : mode = NONE; break;
      default: mode = MEMSET; this->nValue = -nValue; break;
      }
   }

   bool operator==(const Operator &operation) const { return mode == operation.mode; }
   bool operator==(Mode mode) const { return mode == this->mode; }
   operator Mode() const { return mode; }
   int value() const { return nValue; }
};

} } // namespace MaskTools, Filtering

#endif
