// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// #define _HAS_EXCEPTIONS 0  <- can't due to regex.
#define _STATIC_CPPLIB

#include <assert.h>
#include "targetver.h"
#include <stdio.h>

#ifdef _DEBUG
#define ASSERT(expr)  assert(expr)
#else
#define ASSERT(expr)  ((void)0)
#endif

template <typename T, size_t N>
char (&ArraySizeHelper(T (&array)[N]))[N];

#define arraysize(array) (sizeof(ArraySizeHelper(array)))

typedef unsigned char byte;
typedef unsigned __int16 uint16_t;
