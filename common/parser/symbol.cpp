#include "symbol.h"
#include <math.h>

using namespace Filtering;
using namespace Filtering::Parser;

static double addition        (double x, double y, double z) { UNUSED(z); return x + y; }
static double multiplication  (double x, double y, double z) { UNUSED(z); return x * y; }
static double division        (double x, double y, double z) { UNUSED(z); return x / y; }
static double substraction    (double x, double y, double z) { UNUSED(z); return x - y; }
static double power           (double x, double y, double z) { UNUSED(z); return pow(x, y); }
static double modulo          (double x, double y, double z) { UNUSED(z); return double(convert<Int64, double>( x ) % convert<Int64, double>( y )); }
static double interrogation   (double x, double y, double z) { return x > 0 ? y : z; }
static double equal           (double x, double y, double z) { UNUSED(z); return abs(x - y) < 0.000001 ? 1 : -1; }
static double notEqual        (double x, double y, double z) { UNUSED(z); return abs(x - y) >= 0.000001 ? 1 : -1; }
static double inferior        (double x, double y, double z) { UNUSED(z); return x <= y ? 1 : -1; }
static double inferiorStrict  (double x, double y, double z) { UNUSED(z); return x < y ? 1 : -1; }
static double superior        (double x, double y, double z) { UNUSED(z); return x >= y ? 1 : -1; }
static double superiorStrict  (double x, double y, double z) { UNUSED(z); return x > y ? 1 : -1; }
static double and             (double x, double y, double z) { UNUSED(z); return x > 0 && y > 0 ? 1 : -1; }
static double or              (double x, double y, double z) { UNUSED(z); return x > 0 || y > 0 ? 1 : -1; }
static double andNot          (double x, double y, double z) { UNUSED(z); return x > 0 && y <= 0 ? 1 : -1; }
static double xor             (double x, double y, double z) { UNUSED(z); return (x > 0 && y <= 0) || (x <= 0 && y > 0)? 1 : -1; }
static double andUB           (double x, double y, double z) { UNUSED(z); return double(clip<Uint64, double>(x) & clip<Uint64, double>(y)); }
static double orUB            (double x, double y, double z) { UNUSED(z); return double(clip<Uint64, double>(x) | clip<Uint64, double>(y)); }
static double xorUB           (double x, double y, double z) { UNUSED(z); return double(clip<Uint64, double>(x) ^ clip<Uint64, double>(y)); }
static double negateUB        (double x, double y, double z) { UNUSED(y); UNUSED(z); return double(~clip<Uint64, double>(x)); }
static double posshiftUB      (double x, double y, double z) { UNUSED(z); return y >= 0 ? double(clip<Uint64, double>(x) << clip<Int64, double>(y)) : double(clip<Uint64, double>(x) >> clip<Int64, double>(-y)); }
static double negshiftUB      (double x, double y, double z) { UNUSED(z); return y >= 0 ? double(clip<Uint64, double>(x) >> clip<Int64, double>(y)) : double(clip<Uint64, double>(x) << clip<Int64, double>(-y)); }
static double andSB           (double x, double y, double z) { UNUSED(z); return double(clip<Int64, double>(x) & clip<Int64, double>(y)); }
static double orSB            (double x, double y, double z) { UNUSED(z); return double(clip<Int64, double>(x) | clip<Int64, double>(y)); }
static double xorSB           (double x, double y, double z) { UNUSED(z); return double(clip<Int64, double>(x) ^ clip<Int64, double>(y)); }
static double negateSB        (double x, double y, double z) { UNUSED(y); UNUSED(z); return double(~clip<Int64, double>(x)); }
static double posshiftSB      (double x, double y, double z) { UNUSED(z); return y >= 0 ? double(clip<Int64, double>(x) << clip<Int64, double>(y)) : double(clip<Int64, double>(x) >> clip<Int64, double>(-y)); }
static double negshiftSB      (double x, double y, double z) { UNUSED(z); return y >= 0 ? double(clip<Int64, double>(x) >> clip<Int64, double>(y)) : double(clip<Int64, double>(x) << clip<Int64, double>(-y)); }
static double cos             (double x, double y, double z) { UNUSED(y); UNUSED(z); return cos(x); }
static double sin             (double x, double y, double z) { UNUSED(y); UNUSED(z); return sin(x); }
static double tan             (double x, double y, double z) { UNUSED(y); UNUSED(z); return tan(x); }
static double exp             (double x, double y, double z) { UNUSED(y); UNUSED(z); return exp(x); }
static double log             (double x, double y, double z) { UNUSED(y); UNUSED(z); return log(x); }
static double abs             (double x, double y, double z) { UNUSED(y); UNUSED(z); return abs(x); }
static double acos            (double x, double y, double z) { UNUSED(y); UNUSED(z); return acos(x); }
static double asin            (double x, double y, double z) { UNUSED(y); UNUSED(z); return asin(x); }
static double atan            (double x, double y, double z) { UNUSED(y); UNUSED(z); return atan(x); }
static double round           (double x, double y, double z) { UNUSED(y); UNUSED(z); return double(convert<Int64, double>( x )); }
static double clip            (double x, double y, double z) { return clip<double, double>( x, y, z ); }
static double min             (double x, double y, double z) { UNUSED(z); return min<double>( x, y ); }
static double max             (double x, double y, double z) { UNUSED(z); return max<double>( x, y ); }
static double floor           (double x, double y, double z) { UNUSED(y); UNUSED(z); return floor(x); }
static double ceil            (double x, double y, double z) { UNUSED(y); UNUSED(z); return ceil(x); }
static double trunc           (double x, double y, double z) { UNUSED(y); UNUSED(z); return double(Int64(x)); }

Symbol Symbol::Addition       ("+" , OPERATOR, 2, addition);
Symbol Symbol::Multiplication ("*" , OPERATOR, 2, multiplication);
Symbol Symbol::Division       ("/" , OPERATOR, 2, division);
Symbol Symbol::Substraction   ("-" , OPERATOR, 2, substraction);
Symbol Symbol::Power          ("^" , OPERATOR, 2, power);
Symbol Symbol::Modulo         ("%" , OPERATOR, 2, modulo);
Symbol Symbol::Interrogation  ("?" , TERNARY , 3, interrogation);
Symbol Symbol::Equal          ("==", OPERATOR, 2, equal);
Symbol Symbol::Equal2         ("=", OPERATOR, 2, equal);
Symbol Symbol::NotEqual       ("!=", OPERATOR, 2, notEqual);
Symbol Symbol::Inferior       ("<=", OPERATOR, 2, inferior);
Symbol Symbol::InferiorStrict ("<" , OPERATOR, 2, inferiorStrict);
Symbol Symbol::Superior       (">=", OPERATOR, 2, superior);
Symbol Symbol::SuperiorStrict (">" , OPERATOR, 2, superiorStrict);
Symbol Symbol::And            ("&" , OPERATOR, 2, and);
Symbol Symbol::Or             ("|" , OPERATOR, 2, or);
Symbol Symbol::AndNot         ("&!", OPERATOR, 2, andNot);
Symbol Symbol::Xor            ("°" , "@", OPERATOR, 2, xor);
Symbol Symbol::AndUB          ("&u" , OPERATOR, 2, andUB);
Symbol Symbol::OrUB           ("|u" , OPERATOR, 2, orUB);
Symbol Symbol::XorUB          ("°u" , "@u", OPERATOR, 2, xorUB);
Symbol Symbol::NegateUB       ("~u" , FUNCTION, 1, negateUB);
Symbol Symbol::PosShiftUB     ("<<", "<<u", OPERATOR, 2, posshiftUB);
Symbol Symbol::NegShiftUB     (">>", ">>u", OPERATOR, 2, negshiftUB);
Symbol Symbol::AndSB          ("&s" , OPERATOR, 2, andSB);
Symbol Symbol::OrSB           ("|s" , OPERATOR, 2, orSB);
Symbol Symbol::XorSB          ("°s" , "@s", OPERATOR, 2, xorSB);
Symbol Symbol::NegateSB       ("~s" , FUNCTION, 1, negateSB);
Symbol Symbol::PosShiftSB     ("<<s", OPERATOR, 2, posshiftSB);
Symbol Symbol::NegShiftSB     (">>s", OPERATOR, 2, negshiftSB);
Symbol Symbol::Pi             ("pi", 3.1415927, NUMBER  , 0, NULL);
Symbol Symbol::X              ("x" , VARIABLE_X, 0, NULL);  
Symbol Symbol::Y              ("y" , VARIABLE_Y, 0, NULL);
Symbol Symbol::Z              ("z" , VARIABLE_Z, 0, NULL);
Symbol Symbol::Cos            ("cos", FUNCTION, 1, cos);
Symbol Symbol::Sin            ("sin", FUNCTION, 1, sin);
Symbol Symbol::Tan            ("tan", FUNCTION, 1, tan);
Symbol Symbol::Log            ("log", FUNCTION, 1, log);
Symbol Symbol::Exp            ("exp", FUNCTION, 1, exp);
Symbol Symbol::Abs            ("abs", FUNCTION, 1, abs);
Symbol Symbol::Atan           ("atan", FUNCTION, 1, atan);
Symbol Symbol::Acos           ("acos", FUNCTION, 1, acos);
Symbol Symbol::Asin           ("asin", FUNCTION, 1, asin);
Symbol Symbol::Round          ("round", FUNCTION, 1, round);
Symbol Symbol::Clip           ("clip", FUNCTION, 3, clip);
Symbol Symbol::Min            ("min", FUNCTION, 2, min);
Symbol Symbol::Max            ("max", FUNCTION, 2, max);
Symbol Symbol::Ceil           ("ceil", FUNCTION, 1, ceil);
Symbol Symbol::Floor          ("floor", FUNCTION, 1, floor);
Symbol Symbol::Trunc          ("trunc", FUNCTION, 1, trunc);

Symbol::Symbol() :
type(UNDEFINED), value(""), value2("")
{
}

Symbol::Symbol(String value, Type type, int nParameter, Process process) :
type(type), value(value), value2(""), nParameter(nParameter), process(process)
{
   if ( type == NUMBER )
      dValue = atof(value.c_str());
}

Symbol::Symbol(String value, String value2, Type type, int nParameter, Process process) :
type(type), value(value), value2(value2), nParameter(nParameter), process(process)
{
   if ( type == NUMBER )
      dValue = atof(value.c_str());
}

Symbol::Symbol(String value, double dValue, Type type, int nParameter, Process process) :
type(type), value(value), value2(""), nParameter(nParameter), process(process), dValue(dValue)
{
}

void Symbol::setValue(double dValue)
{
   this->dValue = dValue;
}

double Symbol::getValue(double x, double y, double z) const
{
   switch ( type )
   {
   case VARIABLE_X:
   case VARIABLE_Y:
   case VARIABLE_Z:
   case NUMBER:
      return dValue;
   default:
      return process(x, y, z);
   }
}

Context::Context(const std::list<Symbol> &expression)
{
   nPos = -1;
   nSymbols = expression.size();
   pSymbols = new Symbol[nSymbols];

   std::list<Symbol>::const_iterator it = expression.begin();

   for ( int i = 0; i < nSymbols; i++, it++ )
      pSymbols[i] = *it;
}

Context::~Context()
{
   delete[] pSymbols;
}

double Context::rec_compute()
{
   const Symbol &s = pSymbols[--nPos];

   switch ( s.type )
   {
   case Symbol::NUMBER: return s.dValue;
   case Symbol::VARIABLE_X: return x;
   case Symbol::VARIABLE_Y: return y;
   case Symbol::VARIABLE_Z: return z;
   default:
      switch ( s.nParameter )
      {
      case 2 :
         {
            double y = rec_compute();
            double x = rec_compute();
            return s.process(x, y, -1.0f);
         }
      case 1: return s.process(rec_compute(), -1.0f, -1.0f);
      case 3:
         {
            double z = rec_compute();
            double y = rec_compute();
            double x = rec_compute();
            return s.getValue(x, y, z);
         }
      default: return s.process(-1.0f, -1.0f, -1.0f);
      }
   }
}

double Context::compute(double x, double y, double z)
{
   nPos = nSymbols;
   this->x = x;
   this->y = y;
   this->z = z;

   return rec_compute();
}

String Context::rec_infix()
{
   const Symbol &s = pSymbols[--nPos];

   switch ( s.type )
   {
   case Symbol::VARIABLE_X: 
   case Symbol::VARIABLE_Y: 
   case Symbol::VARIABLE_Z: 
   case Symbol::NUMBER: return s.value;
   case Symbol::FUNCTION:
      if (s.nParameter == 1)
         return s.value + "(" + rec_infix() + ")";
      else if (s.nParameter == 2)
         return s.value + "(" + rec_infix() + "," + rec_infix() + ")";
      else
         return s.value + "(" + rec_infix() + "," + rec_infix() + "," + rec_infix() + ")";
   case Symbol::OPERATOR:
      return "(" + rec_infix() + s.value + rec_infix() + ")";
   case Symbol::TERNARY:
      return "((" + rec_infix() + ") ? " + rec_infix() + " : " + rec_infix() + ")";
   default:
      assert(0);
      return "";
   }
}

String Context::infix()
{
   nPos = nSymbols;

   return rec_infix();
}

bool Context::check()
{
   return true;
}