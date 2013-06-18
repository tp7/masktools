#ifndef __Common_Avs2x_Filter_H__
#define __Common_Avs2x_Filter_H__

#include "params.h"

namespace Filtering { namespace Avisynth2x {

template<class T>
class Filter : public GenericVideoFilter
{
   T _filter;
   Signature signature;

   static AVSValue __cdecl _create(AVSValue args, void *user_data, IScriptEnvironment *env)
   {
      UNUSED(user_data);
      Functions::Asm asmLock;
      return new Filter<T>(args[0].AsClip(), GetParameters(args, T::filter_signature(), env), env);
   }
public:
   Filter(::PClip child, const Parameters &parameters, IScriptEnvironment *env) : _filter( parameters ), GenericVideoFilter(child), signature(T::filter_signature())
   {
      if ( _filter.is_error() )
      {
         env->ThrowError( ( signature.getName() + " : " + _filter.get_error() ).c_str() );
      }
   }

   PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment *env)
   {
      PVideoFrame dst = _filter.is_in_place() ? child->GetFrame(n, env) : env->NewVideoFrame(vi);

      if ( _filter.is_in_place() )
         env->MakeWritable( &dst );

      for ( int i = 0; i < int(_filter.get_childs().size()); i++ )
         dynamic_cast<Clip *>( (Filtering::Clip *)_filter.get_childs()[i] )->set_env( env );

      Frame<Byte> destination = dynamic_cast<Clip *>( (Filtering::Clip *)_filter.get_childs()[0] )->ConvertTo<Byte>( dst );

      _filter.get_frame( n, destination );

      for ( int i = 0; i < int(_filter.get_childs().size()); i++ )
         _filter.get_childs()[i]->release_frames();

      return dst;
   }

   static void create(IScriptEnvironment *env)
   {
      env->AddFunction(T::filter_signature().getName().c_str(), SignatureToString( T::filter_signature() ).c_str(), _create, NULL );
   }
};

} } // namespace Avisynth2x, Filtering

#endif
