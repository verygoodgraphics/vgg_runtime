#pragma once
#if defined(_WIN32) && defined(LAYER_SHARED_LIBRARY)
#ifdef layer_EXPORTS
#define VGG_EXPORTS __declspec(dllexport)
#else
#define VGG_EXPORTS __declspec(dllimport)
#endif
#else
#define VGG_EXPORTS
#endif
