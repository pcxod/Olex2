//----------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#ifndef __olx_sdl_defs_H
#define __olx_sdl_defs_H
//---------------------------------------------------------------------------
#ifndef NULL
  #ifdef __cplusplus
    #define NULL    0
  #else
    #define NULL    ((void *)0)
   #endif
#endif
//---------------------------------------------------------------------------
#ifndef olx_min
  #define olx_min(a, b)  (((a) < (b)) ? (a) : (b))
#endif
#ifndef olx_max
  #define olx_max(a, b)  (((a) > (b)) ? (a) : (b))
#endif

#ifndef M_PI
  #define M_PI 3.14159265358979323846
#endif

#ifndef __WIN32__
  #ifdef __WINDOWS__  
    #define __WIN32__ 1
  #endif
#endif
#ifndef __WIN32__
  #ifdef _WINDOWS
    #define __WIN32__ 2
  #endif
#endif
#ifndef __WIN32__
  #ifdef _WIN32
    #define __WIN32__ 3
  #endif
#endif

#ifndef __FUNC__
  #define __FUNC__ __FUNCTION__
#endif

// just a complaint about functions having loops, are not expandable inline
#ifdef __BORLANDC__
  #pragma warn -8027
  #pragma warn -8022  //hidden virtual functions ...
#endif

#ifdef __GNUC__
  #define  __OlxSourceInfo (olxstr(EmptyString(), 384) << "[" __FILE__ << '(' << __FUNC__ << "):" << __LINE__ << ']')
#else
  #define  __OlxSourceInfo (olxstr(EmptyString(), 384) << "["__FILE__"("__FUNC__"):" << __LINE__ << ']')
#endif
#define  __POlxSourceInfo __FILE__,__FUNC__,__LINE__
//#define  __OlxSourceInfo __FILE__" in "__FUNC__
#define  __OlxSrcInfo __FUNC__

#ifdef __GNUC__
  #include <stdint.h>
#endif

#ifdef __WIN32__  //linux
# if defined(_MSC_VER) && (_MSC_VER >= 1600)
    #include <stdint.h>
# else
    typedef __int8  int8_t;
    typedef __int16 int16_t;
    typedef __int32 int32_t;
    typedef __int64 int64_t;
    typedef unsigned __int8  uint8_t;
    typedef unsigned __int16 uint16_t;
    typedef unsigned __int32 uint32_t;
    typedef unsigned __int64 uint64_t;
# endif
# ifdef _WIN64
    typedef __int64 index_t;
# else
    typedef __int32 index_t;
# endif
#else // non Windows
  typedef int32_t index_t;
#endif

#ifdef _UNICODE
  typedef wchar_t olxch;
#define olxT(a) (L##a)
#else
  typedef char olxch;
  #define olxT(a) (a)
#endif

#endif
