#include "common.h"
#include <stdio.h>
#include <windows.h>

namespace Filtering { namespace MaskTools {

LogLevel logLevel = LOG_DEBUG;

void print(LogLevel level, const char *format, ...)
{
   va_list args;
   char buf[1024];

   if ( level <= logLevel )
   {
      va_start(args, format);
      vsprintf(buf, format, args);
      OutputDebugString(buf);
   }
}

} } // namespace MaskTools, Filtering
