
#pragma once

#define USTC_CG_NAMESPACE_OPEN_SCOPE namespace USTC_CG{
#define USTC_CG_NAMESPACE_CLOSE_SCOPE }

#if defined(_MSC_VER)
#  define RHI_EXPORT   __declspec(dllexport)
#  define RHI_IMPORT   __declspec(dllimport)
#  define RHI_NOINLINE __declspec(noinline)
#  define RHI_INLINE   __forceinline
#else
#  define RHI_EXPORT    __attribute__ ((visibility("default")))
#  define RHI_IMPORT
#  define RHI_NOINLINE  __attribute__ ((noinline))
#  define RHI_INLINE    __attribute__((always_inline)) inline
#endif

#if BUILD_RHI_MODULE
#  define RHI_API RHI_EXPORT
#  define RHI_EXTERN extern
#else
#  define RHI_API RHI_IMPORT
#  if defined(_MSC_VER)
#    define RHI_EXTERN
#  else
#    define RHI_EXTERN extern
#  endif
#endif
