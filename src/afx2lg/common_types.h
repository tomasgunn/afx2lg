// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#pragma once

#ifndef COMMON_TYPES_H_
#define COMMON_TYPES_H_

#include <assert.h>

#ifdef WIN32
#ifdef _DEBUG
#define ASSERT(expr)  ((expr) ? ((void)0) : __debugbreak())
#else
#define ASSERT(expr)  ((void)0)
#endif
#else  // !WIN32
#ifdef _DEBUG
#define ASSERT(expr)  assert(expr)
#else
#define ASSERT(expr)  ((void)0)
#endif
#endif

// #define _HAS_EXCEPTIONS 0  <- can't due to regex.
#define _STATIC_CPPLIB

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

template <typename T, size_t N>
char (&ArraySizeHelper(T (&array)[N]))[N];

#define arraysize(array) (sizeof(ArraySizeHelper(array)))

template <bool>
struct CompileAssert {
};

#undef COMPILE_ASSERT
#define COMPILE_ASSERT(expr, msg) \
  typedef CompileAssert<(bool(expr))> msg[bool(expr) ? 1 : -1]

#include <stdint.h>

#endif  // COMMON_TYPES_H_
