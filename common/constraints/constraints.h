#ifndef __Common_Constraint_H__
#define __Common_Constraint_H__

#include "../utils/utils.h"
#include "../plane/plane.h"

namespace Filtering {

enum {

   CPU_NONE    = 0,
   CPU_MMX     = 0x01,
   CPU_ISSE    = 0x02,
   CPU_SSE2    = 0x04,
   CPU_SSE3    = 0x08,
   CPU_3DNOW   = 0x10,
   CPU_3DNOW2  = 0x20,
   CPU_SSSE3   = 0x40,
   CPU_SSE4_1    = 0x80,
   CPU_SSE4_2    = 0x100,
};

typedef int CpuFlags;

typedef enum {

   MODULO_NONE = 0,
   MODULO_2    = 1,
   MODULO_4    = 3,
   MODULO_8    = 7,
   MODULO_16   = 15,
   MODULO_32   = 31,

} Modulo;

Modulo toModulo(Int64 n);

typedef enum {

   ALIGNMENT_NONE = 0,
   ALIGNMENT_2    = 1,
   ALIGNMENT_4    = 3,
   ALIGNMENT_8    = 7,
   ALIGNMENT_16   = 15,
   ALIGNMENT_32   = 31,

} Alignment;

Alignment toAlignment(Int64 n);

class Constraint {

   CpuFlags    flags;
   Modulo      moduloX;
   Modulo      moduloY;
   Alignment   alignment;
   int         nMinimumWidth;

public:

   Constraint();
   Constraint(CpuFlags flags, Modulo moduloX, Modulo moduloY, Alignment alignment, int nMinimumWidth = 16);
   Constraint(CpuFlags flags, int nWidth, int nHeight, const Byte *pAddress, ptrdiff_t nPitch);
   Constraint(CpuFlags flags, int nWidth, int nHeight, Int64 nAddress, ptrdiff_t nPitch);
   Constraint(CpuFlags flags, const Plane<const Byte> &plane);
   Constraint(CpuFlags flags, const Plane<Byte> &plane);
   Constraint(const Constraint &constraint, const Plane<const Byte> &plane);
   Constraint(const Constraint &constraint, const Plane<Byte> &plane);
   bool respect(const Constraint &constraint) const;
   void print() const;
};

template<class P>
class Processor {

   P           *processor;
   int         nSpeed;
   Constraint  constraint;

public:

   Processor() : processor(NULL), nSpeed(-1) { }
   Processor(P *processor) : processor(processor), constraint( CPU_NONE, 1, 1, 1, 1 ), nSpeed( 0 ) { }
   Processor(P *processor, const Constraint &constraint, int nSpeed) : processor(processor), constraint(constraint), nSpeed(nSpeed) { }
   bool respect( const Constraint &constraint) const { return this->constraint.respect(constraint); }
   int speed() const { return nSpeed; }
   P* getFunction() const { return processor; }
   void print() const { constraint.print(); }

};

template<class P>
class ProcessorList : public std::list<Processor<P> >
{

public:

   P *best_processor(const Constraint &constraint) const 
   {
      int nBestSpeed = -1;
      Processor<P> best_proc;

      for(const auto &processor: *this) {
          if (processor.speed() > nBestSpeed && processor.respect(constraint)) {
              nBestSpeed = processor.speed();
              best_proc = processor;
          }
      }

      return best_proc.getFunction();
   }
};

} // namespace Filtering

#endif
