#ifndef __Mt_Symbol_H__
#define __Mt_Symbol_H__

#include "../utils/utils.h"

namespace Filtering { namespace Parser {

class Symbol {
public:
   typedef enum {
      NUMBER,
      OPERATOR,
      FUNCTION,
      TERNARY,
      VARIABLE_X,
      VARIABLE_Y,
      VARIABLE_Z,

      UNDEFINED

   } Type;

public:

   Type type;
   String value;
   String value2;
   int nParameter;
   double dValue;
   typedef double (*Process)(double x, double y, double z);
   Process process;

private:

private:
public:

   Symbol();
   Symbol(String value, Type type, int nParameter, Process process);
   Symbol(String value, String value2, Type type, int nParameter, Process process);
   Symbol(String value, double dValue, Type type, int nParameter, Process process);

   void setValue(double dValue);
   double getValue(double x, double y, double z) const;

   static Symbol Addition;
   static Symbol Multiplication;
   static Symbol Division;
   static Symbol Substraction;
   static Symbol Power;
   static Symbol Modulo;
   static Symbol Interrogation;
   static Symbol Equal;
   static Symbol Equal2;
   static Symbol NotEqual;
   static Symbol Inferior;
   static Symbol InferiorStrict;
   static Symbol Superior;
   static Symbol SuperiorStrict;
   static Symbol And;
   static Symbol Or;
   static Symbol AndNot;
   static Symbol Xor;
   static Symbol AndUB;
   static Symbol OrUB;
   static Symbol XorUB;
   static Symbol NegateUB;
   static Symbol PosShiftUB;
   static Symbol NegShiftUB;
   static Symbol AndSB;
   static Symbol OrSB;
   static Symbol XorSB;
   static Symbol NegateSB;
   static Symbol PosShiftSB;
   static Symbol NegShiftSB;
   static Symbol Pi;
   static Symbol X;
   static Symbol Y;
   static Symbol Z;
   static Symbol Cos;
   static Symbol Sin;
   static Symbol Tan;
   static Symbol Log;
   static Symbol Abs;
   static Symbol Exp;
   static Symbol Acos;
   static Symbol Atan;
   static Symbol Asin;
   static Symbol Round;
   static Symbol Clip;
   static Symbol Min;
   static Symbol Max;
   static Symbol Ceil;
   static Symbol Floor;
   static Symbol Trunc;
};

class Context : private std::list<double> {

   Symbol *pSymbols;
   int nSymbols;
   int nPos;

   double x, y, z;
   double rec_compute();
   String rec_infix();

public:
   
   Context(const std::list<Symbol> &expression);

   ~Context();

   bool check();
   double compute(double x, double y = -1.0f, double z = -1.0f);
   String infix();
   Byte compute_byte(double x, double y = -1.0f, double z = -1.0f) { return clip<Byte, double>( compute(x, y, z) ); }
};

} } // namespace Parser, Filtering

#endif
