#ifndef APEYUSH_SUGARS_H
#define APEYUSH_SUGARS_H
// Sugars ---------------------------------------------------- //

#include <stdint.h>

#define global        static
#define internal      static
#define local_persist static

#define Pi32 3.1415926536f

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float  f32;
typedef double f64;

#define ArrayLen(ARRAY) (sizeof(ARRAY) / sizeof((ARRAY)[0]))

#define KiloBytes(VAL) ((VAL) * (u64)1024)
#define MegaBytes(VAL) (KiloBytes(VAL) * 1024)
#define GigaBytes(VAL) (MegaBytes(VAL) * 1024)
#define TeraBytes(VAL) (GigaBytes(VAL) * 1024)

#define Text(Literal) ((char*)(Literal))

#define Assert(EXP)                                                                                \
    if (!(EXP)) {                                                                                  \
        *(volatile int*)0 = 0;                                                                     \
    };


internal inline u32 SafeTruncateU64(u64 Val) {
    Assert(Val <= 0xffffffff);
    return (u32)Val;
}


internal inline void ZeroMemory(void* Memory, u32 Size) {
    u8* Byte = (u8*)Memory;
    while (Size--) {
        *Byte++ = 0;
    }
}

#define ZeroStruct(Instance)    ZeroMemory(&(Instance), sizeof(Instance))
#define ZeroArray(Array, Count) ZeroMemory((Array), sizeof((Array)[0]) * (Count))

// ----------------------------------------------------------- //
#endif
