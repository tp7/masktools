#include "constraints.h"

namespace Filtering {

Modulo toModulo(Int64 n)
{
   if ( !(n & MODULO_32) )
      return MODULO_32;
   else if ( !(n & MODULO_16) )
      return MODULO_16;
   else if ( !(n & MODULO_8) )
      return MODULO_8;
   else if ( !(n & MODULO_4) )
      return MODULO_4;
   else if ( !(n & MODULO_2) )
      return MODULO_2;
   else
      return MODULO_NONE;
}

static Modulo operator &(Modulo m1, Modulo m2) { return toModulo( ~(static_cast<int>(m1) & static_cast<int>(m2)) ); }

Alignment toAlignment(Int64 n)
{
   if ( !(n & ALIGNMENT_32) )
      return ALIGNMENT_32;
   else if ( !(n & ALIGNMENT_16) )
      return ALIGNMENT_16;
   else if ( !(n & ALIGNMENT_8) )
      return ALIGNMENT_8;
   else if ( !(n & ALIGNMENT_4) )
      return ALIGNMENT_4;
   else if ( !(n & ALIGNMENT_2) )
      return ALIGNMENT_2;
   else
      return ALIGNMENT_NONE;
}

static Alignment operator &(Alignment a1, Alignment a2) { return toAlignment( ~(static_cast<int>(a1) & static_cast<int>(a2)) ); }

Constraint::Constraint() : flags(CPU_NONE) { }

Constraint::Constraint(CpuFlags flags, Modulo moduloX, Modulo moduloY, Alignment alignment, int nMinimumWidth) :
flags(flags), moduloX(moduloX), moduloY(moduloY), alignment(alignment), nMinimumWidth(nMinimumWidth) { }

Constraint::Constraint(CpuFlags flags, int nWidth, int nHeight, const Byte *pAddress, ptrdiff_t nPitch) :
flags(flags), moduloX(toModulo(nWidth)), moduloY(toModulo(nHeight)), alignment(toAlignment(Int64(pAddress) | nPitch)), nMinimumWidth(nWidth) { }

Constraint::Constraint(CpuFlags flags, int nWidth, int nHeight, Int64 nAddress, ptrdiff_t nPitch) :
flags(flags), moduloX(toModulo(nWidth)), moduloY(toModulo(nHeight)), alignment(toAlignment(nAddress | nPitch)), nMinimumWidth(nWidth) { }

Constraint::Constraint(CpuFlags flags, const Plane<const Byte> &plane) :
flags(flags), moduloX(toModulo(plane.width())), moduloY(toModulo(plane.height())), alignment(toAlignment(Int64(static_cast<const Byte*>(plane)) | plane.pitch())), nMinimumWidth(plane.width()) {}

Constraint::Constraint(CpuFlags flags, const Plane<Byte> &plane) :
flags(flags), moduloX(toModulo(plane.width())), moduloY(toModulo(plane.height())), alignment(toAlignment(Int64(static_cast<const Byte*>(plane)) | plane.pitch())), nMinimumWidth(plane.width()) {}

Constraint::Constraint(const Constraint &constraint, const Plane<Byte> &plane) :
flags(constraint.flags), moduloX(constraint.moduloX & toModulo(plane.width())), moduloY(constraint.moduloY & toModulo(plane.height())), alignment(constraint.alignment & toAlignment(Int64(static_cast<const Byte*>(plane)) | plane.pitch())), nMinimumWidth(plane.width()) {}

Constraint::Constraint(const Constraint &constraint, const Plane<const Byte> &plane) :
flags(constraint.flags), moduloX(constraint.moduloX & toModulo(plane.width())), moduloY(constraint.moduloY & toModulo(plane.height())), alignment(constraint.alignment & toAlignment(Int64(static_cast<const Byte*>(plane)) | plane.pitch())), nMinimumWidth(plane.width()) {}

bool Constraint::respect(const Constraint &constraint) const
{
   return ((flags & constraint.flags) == flags) && (moduloX <= constraint.moduloX) && 
          (moduloY <= constraint.moduloY) && (alignment <= constraint.alignment) && (nMinimumWidth <= constraint.nMinimumWidth);
}

void Constraint::print() const
{
   Filtering::print(MAX_LOG_LEVEL, "flags : 0x%02x --- modulo : 0x%02x 0x%02x --- alignment : 0x%02x", flags, moduloX, moduloY, alignment);
}

}
