
#pragma once

#define USTC_CG_NAMESPACE_OPEN_SCOPE namespace USTC_CG{
#define USTC_CG_NAMESPACE_CLOSE_SCOPE }

#if defined(_MSC_VER)
#  define GLINTIFY_EXPORT   __declspec(dllexport)
#  define GLINTIFY_IMPORT   __declspec(dllimport)
#  define GLINTIFY_NOINLINE __declspec(noinline)
#  define GLINTIFY_INLINE   __forceinline
#else
#  define GLINTIFY_EXPORT    __attribute__ ((visibility("default")))
#  define GLINTIFY_IMPORT
#  define GLINTIFY_NOINLINE  __attribute__ ((noinline))
#  define GLINTIFY_INLINE    __attribute__((always_inline)) inline
#endif

#if BUILD_GLINTIFY_MODULE
#  define GLINTIFY_API GLINTIFY_EXPORT
#  define GLINTIFY_EXTERN extern
#else
#  define GLINTIFY_API GLINTIFY_IMPORT
#  if defined(_MSC_VER)
#    define GLINTIFY_EXTERN
#  else
#    define GLINTIFY_EXTERN extern
#  endif
#endif
