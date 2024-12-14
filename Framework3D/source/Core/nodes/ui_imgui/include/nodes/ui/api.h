
#pragma once

#define USTC_CG_NAMESPACE_OPEN_SCOPE namespace USTC_CG{
#define USTC_CG_NAMESPACE_CLOSE_SCOPE }

#if defined(_MSC_VER)
#  define NODES_UI_IMGUI_EXPORT   __declspec(dllexport)
#  define NODES_UI_IMGUI_IMPORT   __declspec(dllimport)
#  define NODES_UI_IMGUI_NOINLINE __declspec(noinline)
#  define NODES_UI_IMGUI_INLINE   __forceinline
#else
#  define NODES_UI_IMGUI_EXPORT    __attribute__ ((visibility("default")))
#  define NODES_UI_IMGUI_IMPORT
#  define NODES_UI_IMGUI_NOINLINE  __attribute__ ((noinline))
#  define NODES_UI_IMGUI_INLINE    __attribute__((always_inline)) inline
#endif

#if BUILD_NODES_UI_IMGUI_MODULE
#  define NODES_UI_IMGUI_API NODES_UI_IMGUI_EXPORT
#  define NODES_UI_IMGUI_EXTERN extern
#else
#  define NODES_UI_IMGUI_API NODES_UI_IMGUI_IMPORT
#  if defined(_MSC_VER)
#    define NODES_UI_IMGUI_EXTERN
#  else
#    define NODES_UI_IMGUI_EXTERN extern
#  endif
#endif
