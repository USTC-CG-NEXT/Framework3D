
#pragma once

#define USTC_CG_NAMESPACE_OPEN_SCOPE namespace USTC_CG{
#define USTC_CG_NAMESPACE_CLOSE_SCOPE }

#if defined(_MSC_VER)
#  define STAGE_EXPORT   __declspec(dllexport)
#  define STAGE_IMPORT   __declspec(dllimport)
#  define STAGE_NOINLINE __declspec(noinline)
#  define STAGE_INLINE   __forceinline
#else
#  define STAGE_EXPORT    __attribute__ ((visibility("default")))
#  define STAGE_IMPORT
#  define STAGE_NOINLINE  __attribute__ ((noinline))
#  define STAGE_INLINE    __attribute__((always_inline)) inline
#endif

#if BUILD_STAGE_MODULE
#  define STAGE_API STAGE_EXPORT
#  define STAGE_EXTERN extern
#else
#  define STAGE_API STAGE_IMPORT
#  if defined(_MSC_VER)
#    define STAGE_EXTERN
#  else
#    define STAGE_EXTERN extern
#  endif
#endif
