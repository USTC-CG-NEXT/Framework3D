
#pragma once

#define USTC_CG_NAMESPACE_OPEN_SCOPE  namespace USTC_CG {
#define USTC_CG_NAMESPACE_CLOSE_SCOPE }

#if defined(_MSC_VER)
#define GUI_EXPORT   __declspec(dllexport)
#define GUI_IMPORT   __declspec(dllimport)
#define GUI_NOINLINE __declspec(noinline)
#define GUI_INLINE   __forceinline
#else
#define GUI_EXPORT __attribute__((visibility("default")))
#define GUI_IMPORT
#define GUI_NOINLINE __attribute__((noinline))
#define GUI_INLINE   __attribute__((always_inline)) inline
#endif

#if BUILD_GUI_MODULE
#define GUI_API    GUI_EXPORT
#define GUI_EXTERN extern
#else
#define GUI_API GUI_IMPORT
#if defined(_MSC_VER)
#define GUI_EXTERN
#else
#define GUI_EXTERN extern
#endif
#endif
