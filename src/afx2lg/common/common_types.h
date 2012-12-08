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

// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&);             \
    void operator=(const TypeName&)

#include <stdint.h>

#endif  // COMMON_TYPES_H_
