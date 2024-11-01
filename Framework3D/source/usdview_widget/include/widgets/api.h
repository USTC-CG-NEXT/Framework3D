
#pragma once

#define USTC_CG_NAMESPACE_OPEN_SCOPE namespace USTC_CG{
#define USTC_CG_NAMESPACE_CLOSE_SCOPE }

#if defined(_MSC_VER)
#  define USDVIEW_WIDGET_EXPORT   __declspec(dllexport)
#  define USDVIEW_WIDGET_IMPORT   __declspec(dllimport)
#  define USDVIEW_WIDGET_NOINLINE __declspec(noinline)
#  define USDVIEW_WIDGET_INLINE   __forceinline
#else
#  define USDVIEW_WIDGET_EXPORT    __attribute__ ((visibility("default")))
#  define USDVIEW_WIDGET_IMPORT
#  define USDVIEW_WIDGET_NOINLINE  __attribute__ ((noinline))
#  define USDVIEW_WIDGET_INLINE    __attribute__((always_inline)) inline
#endif

#if BUILD_USDVIEW_WIDGET_MODULE
#  define USDVIEW_WIDGET_API USDVIEW_WIDGET_EXPORT
#  define USDVIEW_WIDGET_EXTERN extern
#else
#  define USDVIEW_WIDGET_API USDVIEW_WIDGET_IMPORT
#  if defined(_MSC_VER)
#    define USDVIEW_WIDGET_EXTERN
#  else
#    define USDVIEW_WIDGET_EXTERN extern
#  endif
#endif
