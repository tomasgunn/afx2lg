// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#pragma once

#ifndef COMMON_TYPES_H_
#define COMMON_TYPES_H_

#include <assert.h>
#include <cstring>  // size_t

#if defined(__APPLE__)
#define OS_MACOSX 1
#elif defined(ANDROID)
#define OS_ANDROID 1
#elif defined(__linux__)
#define OS_LINUX 1
#elif defined(_WIN32)
#define OS_WIN 1
#else
#error Platform not supported.
#endif

#ifdef OS_WIN
#ifndef NDEBUG
#define ASSERT(expr)  ((expr) ? ((void)0) : __debugbreak())
#else
#define ASSERT(expr)  ((void)0)
#endif
#else  // !OS_WIN
#ifndef NDEBUG
#define ASSERT(expr)  assert(expr)
#else
#define ASSERT(expr)  ((void)0)
#endif
#endif

// #define _HAS_EXCEPTIONS 0  <- can't due to regex.
#define _STATIC_CPPLIB

#if defined(OS_WIN) && !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x0600
#endif

template <typename T, size_t N>
char (&ArraySizeHelper(T (&array)[N]))[N];
#define arraysize(array) (sizeof(ArraySizeHelper(array)))

// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&);             \
    void operator=(const TypeName&)

#include <stdint.h>

// Pull shared_ptr and unique_ptr into the global namespace.
// We use these pretty heavily, so let's do it once and for all.
#include <memory>
using std::shared_ptr;
using std::unique_ptr;

#endif  // COMMON_TYPES_H_
