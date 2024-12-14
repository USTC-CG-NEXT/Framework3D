
#pragma once

#define USTC_CG_NAMESPACE_OPEN_SCOPE namespace USTC_CG{
#define USTC_CG_NAMESPACE_CLOSE_SCOPE }

#if defined(_MSC_VER)
#  define DIFF_OPTICS_EXPORT   __declspec(dllexport)
#  define DIFF_OPTICS_IMPORT   __declspec(dllimport)
#  define DIFF_OPTICS_NOINLINE __declspec(noinline)
#  define DIFF_OPTICS_INLINE   __forceinline
#else
#  define DIFF_OPTICS_EXPORT    __attribute__ ((visibility("default")))
#  define DIFF_OPTICS_IMPORT
#  define DIFF_OPTICS_NOINLINE  __attribute__ ((noinline))
#  define DIFF_OPTICS_INLINE    __attribute__((always_inline)) inline
#endif

#if BUILD_DIFF_OPTICS_MODULE
#  define DIFF_OPTICS_API DIFF_OPTICS_EXPORT
#  define DIFF_OPTICS_EXTERN extern
#else
#  define DIFF_OPTICS_API DIFF_OPTICS_IMPORT
#  if defined(_MSC_VER)
#    define DIFF_OPTICS_EXTERN
#  else
#    define DIFF_OPTICS_EXTERN extern
#  endif
#endif
