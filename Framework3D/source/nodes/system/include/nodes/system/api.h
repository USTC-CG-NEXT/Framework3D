
#pragma once

#define USTC_CG_NAMESPACE_OPEN_SCOPE namespace USTC_CG{
#define USTC_CG_NAMESPACE_CLOSE_SCOPE }

#if defined(_MSC_VER)
#  define NODES_SYSTEM_EXPORT   __declspec(dllexport)
#  define NODES_SYSTEM_IMPORT   __declspec(dllimport)
#  define NODES_SYSTEM_NOINLINE __declspec(noinline)
#  define NODES_SYSTEM_INLINE   __forceinline
#else
#  define NODES_SYSTEM_EXPORT    __attribute__ ((visibility("default")))
#  define NODES_SYSTEM_IMPORT
#  define NODES_SYSTEM_NOINLINE  __attribute__ ((noinline))
#  define NODES_SYSTEM_INLINE    __attribute__((always_inline)) inline
#endif

#if BUILD_NODES_SYSTEM_MODULE
#  define NODES_SYSTEM_API NODES_SYSTEM_EXPORT
#  define NODES_SYSTEM_EXTERN extern
#else
#  define NODES_SYSTEM_API NODES_SYSTEM_IMPORT
#  if defined(_MSC_VER)
#    define NODES_SYSTEM_EXTERN
#  else
#    define NODES_SYSTEM_EXTERN extern
#  endif
#endif
