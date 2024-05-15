#pragma once

// Here we create a number of defines to make it easy to choose between
// different compilers, operatings systems and CPU architectures.
// Some information about the defines used can be found here:
// http://sourceforge.net/p/predef/wiki/Architectures/

// Detect operating systems
#if defined(__linux__)
#define PLATFORM_LINUX 1
#if defined(__ANDROID__)
#define PLATFORM_ANDROID 1
#endif
#elif defined(_WIN32)
#define PLATFORM_WINDOWS 1
#if defined(WINAPI_FAMILY)
#include <winapifamily.h>
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_PHONE_APP)
#define PLATFORM_WINDOWS_PHONE 1
#endif
#endif
#elif defined(__APPLE__)
// Detect iOS before MacOSX (__MACH__ is also defined for iOS)
#if defined(IPHONE)
#define PLATFORM_IOS 1
#elif defined(__MACH__)
#define PLATFORM_MAC 1
#endif
#elif defined(__EMSCRIPTEN__)
#define PLATFORM_EMSCRIPTEN 1
#else
#error "Unable to determine operating system"
#endif

// Detect compilers and CPU architectures
// Note: clang also defines __GNUC__ since it aims to be compatible with GCC.
// Therefore we need to check for __clang__ or __llvm__ first.
#if defined(__clang__) || defined(__llvm__)
#define PLATFORM_CLANG 1
#define PLATFORM_GCC_COMPATIBLE 1
#if defined(__i386__) || defined(__x86_64__)
#define PLATFORM_X86 1
#define PLATFORM_CLANG_X86 1
#define PLATFORM_GCC_COMPATIBLE_X86 1
#elif defined(__arm__) || defined(__arm64__) || defined(__aarch64__)
#define PLATFORM_ARM 1
#define PLATFORM_CLANG_ARM 1
#define PLATFORM_GCC_COMPATIBLE_ARM 1
#elif defined(__mips__)
#define PLATFORM_MIPS 1
#define PLATFORM_CLANG_MIPS 1
#define PLATFORM_GCC_COMPATIBLE_MIPS 1
#elif defined(__asmjs__)
#define PLATFORM_ASMJS 1
#define PLATFORM_CLANG_ASMJS 1
#define PLATFORM_GCC_COMPATIBLE_ASMJS 1
#endif
#elif defined(__GNUC__)
#define PLATFORM_GCC 1
#define PLATFORM_GCC_COMPATIBLE 1
#if defined(__i386__) || defined(__x86_64__)
#define PLATFORM_X86 1
#define PLATFORM_GCC_X86 1
#define PLATFORM_GCC_COMPATIBLE_X86 1
#elif defined(__arm__) || defined(__arm64__) || defined(__aarch64__)
#define PLATFORM_ARM 1
#define PLATFORM_GCC_ARM 1
#define PLATFORM_GCC_COMPATIBLE_ARM 1
#elif defined(__mips__)
#define PLATFORM_MIPS 1
#define PLATFORM_GCC_MIPS 1
#define PLATFORM_GCC_COMPATIBLE_MIPS 1
#endif
#elif defined(_MSC_VER)
#define PLATFORM_MSVC 1
#if defined(_M_IX86) || defined(_M_X64)
#define PLATFORM_X86 1
#define PLATFORM_MSVC_X86 1
#elif defined(_M_ARM) || defined(_M_ARMT)
#define PLATFORM_ARM 1
#define PLATFORM_MSVC_ARM 1
#endif
#else
#error "Unable to determine compiler"
#endif

// Define macros for supported CPU instruction sets
#if defined(PLATFORM_GCC_COMPATIBLE)
#if defined(__MMX__)
#define PLATFORM_MMX 1
#endif
#if defined(__SSE__)
#define PLATFORM_SSE 1
#endif
#if defined(__SSE2__)
#define PLATFORM_SSE2 1
#endif
#if defined(__SSE3__)
#define PLATFORM_SSE3 1
#endif
#if defined(__SSSE3__)
#define PLATFORM_SSSE3 1
#endif
#if defined(__SSE4_1__)
#define PLATFORM_SSE41 1
#endif
#if defined(__SSE4_2__)
#define PLATFORM_SSE42 1
#endif
#if defined(__PCLMUL__)
#define PLATFORM_PCLMUL 1
#endif
#if defined(__AVX__)
#define PLATFORM_AVX 1
#endif
#if defined(__AVX2__)
#define PLATFORM_AVX2 1
#endif
#if defined(__ARM_NEON__) || defined(__ARM_NEON)
#define PLATFORM_NEON 1
#endif
// First, check the PLATFORM_WINDOWS_PHONE define, because
// the X86 instructions sets are not supported on the Windows Phone emulator
#elif defined(PLATFORM_WINDOWS_PHONE)
#if defined(PLATFORM_MSVC_ARM)
// NEON introduced in VS2012
#if (_MSC_VER >= 1700)
#define PLATFORM_NEON 1
#endif
#endif
#elif defined(PLATFORM_MSVC_X86)
// MMX, SSE and SSE2 introduced in VS2003
#if (_MSC_VER >= 1310)
#define PLATFORM_MMX 1
#define PLATFORM_SSE 1
#define PLATFORM_SSE2 1
#endif
// SSE3 introduced in VS2005
#if (_MSC_VER >= 1400)
#define PLATFORM_SSE3 1
#endif
// SSSE3, SSE4.1, SSE4.2, PCLMUL introduced in VS2008
#if (_MSC_VER >= 1500)
#define PLATFORM_SSSE3 1
#define PLATFORM_SSE41 1
#define PLATFORM_SSE42 1
#define PLATFORM_PCLMUL 1
#endif
// AVX and AVX2 introduced in VS2012
#if (_MSC_VER >= 1700)
#define PLATFORM_AVX 1
#define PLATFORM_AVX2 1
#endif
#endif