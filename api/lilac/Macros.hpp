#pragma once

// Set dllexport/dllimport to lilac classes & functions
#ifdef _EXPORTING
   #define LILAC_DLL    __declspec(dllexport)
#else
   #define LILAC_DLL    __declspec(dllimport)
#endif

// Win32
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    #define LILAC_WIN32(...) __VA_ARGS__
    #define LILAC_IS_WIN32
    #define LILAC_IS_DESKTOP
    #define LILAC_PLATFORM_NAME "Win32"
#else
    #define LILAC_WIN32(...)
#endif

// Apple / MacOS
#if defined(__APPLE__)
    #define LILAC_MACOS(...) __VA_ARGS__
    #define LILAC_IS_MACOS
    #define LILAC_IS_DESKTOP
    #define LILAC_PLATFORM_NAME "MacOS"
#else
    #define LILAC_MACOS(...)
#endif

// Android
#ifdef __ANDROID__
    #define LILAC_ANDROID(...) __VA_ARGS__
    #define LILAC_IS_ANDROID
    #define LILAC_IS_MOBILE
    #define LILAC_PLATFORM_NAME "Android"
#else
    #define LILAC_ANDROID(...)
#endif

#ifndef LILAC_PLATFORM_NAME
#error "Unsupported platform!"
#endif

// because C++ doesn't like using a
// namespace that doesn't exist

namespace lilac {}
namespace lilac::cast {}
namespace lilac::cocos {}
namespace lilac::utils {}
namespace lilac::node {}
namespace lilac::op {}
namespace lilac::stream {}
namespace cocos2d {}
namespace cocos2d::extension {}

#define USE_LILAC_NAMESPACE()           \
    using namespace lilac;              \
    using namespace lilac::cast;        \
    using namespace lilac::cocos;       \
    using namespace lilac::utils;       \
    using namespace lilac::node;        \
    using namespace lilac::op;          \
    using namespace lilac::stream;      \
    using namespace cocos2d;            \
    using namespace cocos2d::extension; \


