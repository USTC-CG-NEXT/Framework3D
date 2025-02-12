#pragma once

#define USTC_CG_NAMESPACE_OPEN_SCOPE  namespace USTC_CG {
#define USTC_CG_NAMESPACE_CLOSE_SCOPE }

#if defined(_MSC_VER)
#define POLYSCOPE_WIDGET_EXPORT   __declspec(dllexport)
#define POLYSCOPE_WIDGET_IMPORT   __declspec(dllimport)
#define POLYSCOPE_WIDGET_NOINLINE __declspec(noinline)
#define POLYSCOPE_WIDGET_INLINE   __forceinline
#else
#define POLYSCOPE_WIDGET_EXPORT __attribute__((visibility("default")))
#define POLYSCOPE_WIDGET_IMPORT
#define POLYSCOPE_WIDGET_NOINLINE __attribute__((noinline))
#define POLYSCOPE_WIDGET_INLINE   __attribute__((always_inline)) inline
#endif

#if BUILD_POLYSCOPE_WIDGET_MODULE
#define POLYSCOPE_WIDGET_API    POLYSCOPE_WIDGET_EXPORT
#define POLYSCOPE_WIDGET_EXTERN extern
#else
#define POLYSCOPE_WIDGET_API POLYSCOPE_WIDGET_IMPORT
#if defined(_MSC_VER)
#define POLYSCOPE_WIDGET_EXTERN
#else
#define POLYSCOPE_WIDGET_EXTERN extern
#endif
#endif
