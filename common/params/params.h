#ifndef __Common_Params_H__
#define __Common_Params_H__

#include "../clip/clip.h"
#include "../constraints/constraints.h"

namespace Filtering {

typedef enum {

   TYPE_INT,
   TYPE_FLOAT,
   TYPE_STRING,
   TYPE_BOOL,
   TYPE_CLIP,

   TYPE_UNDEFINED,

} Type;

/* generic """polymorphic""" class, whose value can be defined or not */
class Value {

   Type     type;
   bool     defined;

   PClip    val_clip;
   int      val_int;
   String   val_string;
   bool     val_bool;
   double   val_float;

public:

   Value() : type(TYPE_UNDEFINED), defined(false) { };
   Value(int n) : type(TYPE_INT), defined(true), val_int(n) { }
   Value(double d) : type(TYPE_FLOAT), defined(true), val_float(d) { }
   Value(float d) : type(TYPE_FLOAT), defined(true), val_float(d) { }
   Value(const String &s) : type(TYPE_STRING), defined(true), val_string(s) { }
   Value(bool b) : type(TYPE_BOOL), defined(true), val_bool(b) { }
   Value(const PClip& c) : type(TYPE_CLIP), defined(true), val_clip(c) { }
   Value(Type t) : type(t), defined(false) { }
   Value(const Value &v) : type(v.type), val_clip(v.val_clip), val_int(v.val_int), val_float(v.val_float),
                           val_string(v.val_string), val_bool(v.val_bool), defined(v.defined) { }

   operator int() const { return val_int; }
   operator double() const { return val_float; }
   operator float() const { return float(val_float); }
   operator bool() const { return val_bool; }
   operator String() const { return val_string; }
   operator PClip() const { return val_clip; }
   operator Type() const { return type; }

   int toInt() const { return val_int; }
   double toFloat() const { return val_float; }
   bool toBool() const { return val_bool; }
   String toString() const { return val_string; }
   PClip toClip() const { return val_clip; }

   bool is_defined() const { return defined; }
   void set_defined(bool d) { defined = d; }

};

/* parameter will be used to defined a filter signature, but also to contain the actual value of
   a parameter once the filter has been called */
class Parameter {

   Value       value;
   String      name;

public:

   Parameter() : value(), name("") { }
   Parameter(Type type) : value(type), name("") { }
   Parameter(const Value &value) : value(value), name("") { }
   Parameter(const Value &value, const String &name) : value(value), name(name) { }

   void set_defined(bool d) { value.set_defined( d ); }
   operator String() const { return name; }
   operator Value() const { return value; }
   operator Type() const { return value; }   

};

/* list of parameters */
class Parameters : public std::vector<Parameter> {

public:

   /* handy accessor */
   Value operator[](const String &name) const
   {
      for ( const_iterator it = begin(); it != end(); it++ )
         if ( name == String(*it) )
            return *it;

      return Value();
   }
   Value operator[](int n) const
   {
      return std::vector<Parameter>::operator [](n);
   }

};

/* signature of the filter */
class Signature {

   Parameters              parameters;
   String                  name;

public:

   Signature(const String &name) : name(name) { }
   void add(const Parameter &parameter) { parameters.push_back(parameter); }

   String toString() const { return name; }
   operator String() const { return name; }
   Value operator[](const String &name) const { return parameters[name]; }
   Parameter operator[](int index) const { return parameters.at(index); }
   int count() const { return int(parameters.size()); }

};

} // namespace Filtering

#endif
