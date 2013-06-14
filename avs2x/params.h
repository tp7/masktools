#ifndef __Common_Avs2x_Params_H__
#define __Common_Avs2x_Params_H__

#include "clip.h"

namespace Filtering { namespace Avisynth2x {

Value AVSValueToValue(const AVSValue& value, const Parameter &param)
{
   switch ( Type( Value( param ) ) )
   {
   case TYPE_INT: return Value( value.AsInt() );
   case TYPE_STRING: return Value( String( value.AsString() ) );
   case TYPE_BOOL: return Value( value.AsBool() );
   case TYPE_CLIP: return SmartPointer<Filtering::Clip>(new Clip( value.AsClip() ));
   case TYPE_FLOAT: return Value( value.AsFloat() );
   default: return Value();
   }
}

String ParameterToString(const Parameter &parameter)
{
   String str = "";

   if ( String( parameter ).length() )
      str.append( "[" ).append( parameter ).append( "]" );

   switch ( Type(parameter) )
   {
   case TYPE_INT      : return str.append("i");
   case TYPE_FLOAT    : return str.append("f");
   case TYPE_STRING   : return str.append("s");
   case TYPE_CLIP     : return str.append("c");
   case TYPE_BOOL     : return str.append("b");
   default: assert( 0 ); return "";
   }
}

String SignatureToString(const Signature &signature)
{
   String str;

   for ( int i = 0; i < signature.count(); i++ )
      str.append( ParameterToString( signature[i] ) );

   return str;
}

Parameter GetParameter(const AVSValue &value, const Parameter &default_param)
{
   if ( value.Defined() )
      return Parameter( AVSValueToValue( value, default_param ), String( default_param ) );

   Parameter parameter = default_param;
   parameter.set_defined(false);

   return parameter;
}

Parameters GetParameters(const AVSValue &args, const Signature &signature, IScriptEnvironment *env)
{
   Parameters parameters;

   UNUSED(env);

   for ( int i = 0; i < signature.count(); i++ )
      parameters.push_back( GetParameter( args[i], signature[i] ) );

   return parameters;
}

} }

#endif
