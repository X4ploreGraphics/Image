

#ifndef  NITROS_IMAGE_EXPORT_H
#define  NITROS_IMAGE_EXPORT_H

#if defined (WIN32) && !defined(NITROS_IMAGE_STATIC)
  #if defined(NitrosImage_EXPORTS)
    #define  NIMAGE_EXPORT __declspec(dllexport)
  #else
    #define  NIMAGE_EXPORT __declspec(dllimport)
  #endif 
#else /* defined (WIN32) */
 #define NIMAGE_EXPORT
#endif

#ifndef PLATFORMS_DEFINE
#define PLATFORMS_DEFINE

#if defined (WIN32) 
  #define PLATFORM_WINDOWS
#elif defined(__ANDROID__)
  #define PLATFORM_ANDROID
#elif defined(__EMSCRIPTEN__)
  #define PLATFORM_EMSCRIPTEN
#else 
 #define PLATFORM_LINUX
#endif

#endif

#endif