#ifndef VGG_RUNTIME_DLL_DECLARE

#ifdef _MSC_VER
#ifdef VGG_RUNTIME_DLL_EXPORTING
#define VGG_RUNTIME_DLL_DECLARE __declspec(dllexport)
#else
#define VGG_RUNTIME_DLL_DECLARE __declspec(dllimport)
#endif
#else
#define VGG_RUNTIME_DLL_DECLARE
#endif

#endif