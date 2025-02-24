
#pragma once

#define USTC_CG_NAMESPACE_OPEN_SCOPE namespace USTC_CG{
#define USTC_CG_NAMESPACE_CLOSE_SCOPE }

#if defined(_MSC_VER)
#  define GEOMETRY_EXPORT   __declspec(dllexport)
#  define GEOMETRY_IMPORT   __declspec(dllimport)
#  define GEOMETRY_NOINLINE __declspec(noinline)
#  define GEOMETRY_INLINE   __forceinline
#else
#  define GEOMETRY_EXPORT    __attribute__ ((visibility("default")))
#  define GEOMETRY_IMPORT
#  define GEOMETRY_NOINLINE  __attribute__ ((noinline))
#  define GEOMETRY_INLINE    __attribute__((always_inline)) inline
#endif

#if BUILD_GEOMETRY_MODULE
#  define GEOMETRY_API GEOMETRY_EXPORT
#  define GEOMETRY_EXTERN extern
#else
#  define GEOMETRY_API GEOMETRY_IMPORT
#  if defined(_MSC_VER)
#    define GEOMETRY_EXTERN
#  else
#    define GEOMETRY_EXTERN extern
#  endif
#endif
