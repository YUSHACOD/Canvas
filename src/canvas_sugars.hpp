#ifndef CANVAS_SUGARS_H
#define CANVAS_SUGARS_H

#include <cstdint>

// Sugars ---------------------------------------------------- //
#define global static
#define internal static
#define local_persist static

#define Pi32 3.1415926536f

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

#define ArrayLen(ARRAY) (sizeof(ARRAY) / sizeof((ARRAY)[0]))
// Sugars ---------------------------------------------------- //

#endif
