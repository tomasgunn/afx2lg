// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#pragma once

#ifndef COMMON_TYPES_H_
#define COMMON_TYPES_H_

#include <assert.h>

#ifdef _DEBUG
#define ASSERT(expr)  assert(expr)
#else
#define ASSERT(expr)  ((void)0)
#endif

// #define _HAS_EXCEPTIONS 0  <- can't due to regex.
#define _STATIC_CPPLIB

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

template <typename T, size_t N>
char (&ArraySizeHelper(T (&array)[N]))[N];

#define arraysize(array) (sizeof(ArraySizeHelper(array)))

typedef unsigned char byte;
typedef unsigned __int16 uint16_t;

/*
// Common STL headers.
#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace std {
// merge the tr1 namespace into std for convenience.
using namespace tr1;
}*/

#endif  // COMMON_TYPES_H_
