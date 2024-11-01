
#pragma once

#define USTC_CG_NAMESPACE_OPEN_SCOPE namespace USTC_CG{
#define USTC_CG_NAMESPACE_CLOSE_SCOPE }

#if defined(_MSC_VER)
#  define LOGGER_EXPORT   __declspec(dllexport)
#  define LOGGER_IMPORT   __declspec(dllimport)
#  define LOGGER_NOINLINE __declspec(noinline)
#  define LOGGER_INLINE   __forceinline
#else
#  define LOGGER_EXPORT    __attribute__ ((visibility("default")))
#  define LOGGER_IMPORT
#  define LOGGER_NOINLINE  __attribute__ ((noinline))
#  define LOGGER_INLINE    __attribute__((always_inline)) inline
#endif


#if BUILD_LOGGER_MODULE
#  define LOGGER_API LOGGER_EXPORT
#  define LOGGER_EXTERN extern
#else
#  define LOGGER_API LOGGER_IMPORT
#  if defined(_MSC_VER)
#    define LOGGER_EXTERN
#  else
#    define LOGGER_EXTERN extern
#  endif
#endif
