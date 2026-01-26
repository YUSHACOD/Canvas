#ifndef CANVAS_SUGARS_H
#define CANVAS_SUGARS_H
// Sugars ---------------------------------------------------- //

#include <stdint.h>

#define global        static
#define internal      static
#define local_persist static

#define Pi32 3.1415926536f

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float  float32;
typedef double float64;

#define ArrayLen(ARRAY) (sizeof(ARRAY) / sizeof((ARRAY)[0]))

#define KiloBytes(VAL) ((VAL) * (uint64)1024)
#define MegaBytes(VAL) (KiloBytes(VAL) * 1024)
#define GigaBytes(VAL) (MegaBytes(VAL) * 1024)
#define TeraBytes(VAL) (GigaBytes(VAL) * 1024)

#define Text(Literal) ((char*)(Literal))

#define Assert(EXP)                                                                                \
    if (!(EXP)) {                                                                                  \
        *(volatile int*)0 = 0;                                                                     \
    };


internal inline uint32 SafeTruncateU64(uint64 Val) {
    Assert(Val <= 0xffffffff);
    return (uint32)Val;
}


internal inline void ZeroMemory(void* Memory, uint32 Size) {
    uint8* Byte = (uint8*)Memory;
    while (Size--) {
        *Byte++ = 0;
    }
}

#define ZeroStruct(Instance)    ZeroMemory(&(Instance), sizeof(Instance))
#define ZeroArray(Array, Count) ZeroMemory((Array), sizeof((Array)[0]) * (Count))

// ----------------------------------------------------------- //
#endif
