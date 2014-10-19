#ifndef __EXPLODE_PROPER_EXPORT_HH__
#define __EXPLODE_PROPER_EXPORT_HH__

# if defined(__BEOS__) || defined(__HAIKU__)
#  if defined(__GNUC__)
#   define EXPLODE_PROPER_EXPORT __declspec(dllexport)
#  else
#   define EXPLODE_PROPER_EXPORT __declspec(export)
#  endif
# elif defined(__WIN32__) || defined(_WIN32) || defined (WIN32) || defined (WIN64) || defined (_WIN64)
#  ifdef __BORLANDC__
#    define EXPLODE_PROPER_EXPORT
#    define EXPLODE_PROPER_IMPORT     __declspec(dllimport)
#  else
#   define EXPLODE_PROPER_EXPORT      __declspec(dllexport)
#   define EXPLODE_PROPER_IMPORT      __declspec(dllimport)
#  endif
# elif defined(__OS2__)
#  ifdef __WATCOMC__
#    define EXPLODE_PROPER_EXPORT     __declspec(dllexport)
#    define EXPLODE_PROPER_IMPORT
#  elif defined (__GNUC__) && __GNUC__ < 4
#    define EXPLODE_PROPER_EXPORT    __declspec(dllexport)
#    define EXPLODE_PROPER_IMPORT
#  else
#   define EXPLODE_PROPER_EXPORT
#   define EXPLODE_PROPER_IMPORT
#  endif
# else
#  if defined(__GNUC__) && __GNUC__ >= 4
#   define EXPLODE_PROPER_EXPORT      __attribute__ ((visibility("default")))
#   define EXPLODE_PROPER_IMPORT      __attribute__ ((visibility ("default")))
#  else
#   define EXPLODE_PROPER_EXPORT
#   define EXPLODE_PROPER_IMPORT
#  endif
# endif


#if !defined(EXPLODE_PROPER_IMPORT)
#error "EXPLODE_PROPER_EXPORT is undefined"
#endif

#if defined(BUILD_MZEXPLODE_AS_STATIC_LIB)
#define EXPLODE_API 
#else
#if defined(BUILD_MZEXPLODE_AS_SHARED_LIB)
#define EXPLODE_API EXPLODE_PROPER_EXPORT
#else
#define EXPLODE_API EXPLODE_PROPER_IMPORT
#endif
#endif

#endif
