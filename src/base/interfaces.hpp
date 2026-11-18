#ifndef APE_SUGARS_H
#define APE_SUGARS_H
//  apes stdlib : -------------------------------------------------------------------- (section)  //


#include <stdint.h>
#include <stddef.h>
#include <stdint.h>


#define global static
#define internal
#define local_persist static

#define Glue_(A, B) A##B
#define Glue(A, B)  Glue_(A, B)

#define NoOp                ((void)0)
#define StaticAssert(C, ID) global u8 Glue(ID, __LINE__)[(C) ? 1 : -1]


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


#define ARRAY_DEF(T)                                                                               \
    struct {                                                                                       \
        T*  v;                                                                                     \
        i64 size;                                                                                  \
    }
typedef ARRAY_DEF(i8) I8Array;
typedef ARRAY_DEF(i16) I16Array;
typedef ARRAY_DEF(i32) I32Array;
typedef ARRAY_DEF(i64) I64Array;

typedef ARRAY_DEF(u8) U8Array;
typedef ARRAY_DEF(u16) U16Array;
typedef ARRAY_DEF(u32) U32Array;
typedef ARRAY_DEF(u64) U64Array;
#undef ARRAY_DEF

#define Swap(T, a, b)                                                                              \
    do {                                                                                           \
        T t__ = a;                                                                                 \
        a     = b;                                                                                 \
        b     = t__;                                                                               \
    } while (0)


#define DeferLoop(begin, end) for (int _i_ = ((begin), 0); !_i_; _i_ += 1, (end))
#define DeferLoopChecked(begin, end)                                                               \
    for (int _i_ = 2 * !(begin); (_i_ == 2 ? ((end), 0) : !_i_); _i_ += 1, (end))



#define EnumCount(type) _COUNT_##type
#define Enum(name, ...)                                                                            \
    typedef enum {                                                                                 \
        __VA_ARGS__,                                                                               \
        EnumCount(name),                                                                           \
    } name;


#define EachIndex(it, count)         (U64 it = 0; it < (count); it += 1)
#define EachElement(it, array)       (U64 it = 0; it < ArrayCount(array); it += 1)
#define EachEnumVal(type, it)        (type it = (type)0; it < _COUNT_##type; it = (type)(it + 1))
#define EachNonZeroEnumVal(type, it) (type it = (type)1; it < _COUNT_##type; it = (type)(it + 1))
#define EachInRange(it, range)       (U64 it = (range).min; it < (range).max; it += 1)
#define EachNode(it, T, first)       (T* it = first; it != 0; it = it->next)



//  Math Utils : --------------------------------------------------------------------- (section)  //

// constants
#define Pi32 3.1415926536f

//~ rjf: Clamps, Mins, Maxes

#define Min(A, B)      (((A) < (B)) ? (A) : (B))
#define Max(A, B)      (((A) > (B)) ? (A) : (B))
#define ClampTop(A, X) Min(A, X)
#define ClampBot(X, B) Max(X, B)
#define Clamp(A, X, B) (((X) < (A)) ? (A) : ((X) > (B)) ? (B) : (X))

#define GL_MAT(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p)                                     \
    {a, e, i, m, b, f, j, n, c, g, k, o, d, h, l, p}


#define PercentOf(val, of) ((val / 100.0f) * of)
#define DegToRad(v)        (v * (Pi32 / 180.0f))



//  vec4(math) : --------------------------------------------------------------------- (section)  //
typedef union {
    struct {
        f32 x, y, z, w;
    };
    struct {
        f32 r, g, b, a;
    };
    f32 arr[4];
} v4;
StaticAssert(sizeof(v4) == (sizeof(f32) * 4), v4_size_check_eq_4);

#define V4_arri(name, a0, a1, a2, a3)                                                              \
    name.arr[0] = a0;                                                                              \
    name.arr[1] = a1;                                                                              \
    name.arr[2] = a2;                                                                              \
    name.arr[3] = a3

#define V4_veci(name, X, Y, Z, W)                                                                  \
    name.x = X;                                                                                    \
    name.y = Y;                                                                                    \
    name.z = Z;                                                                                    \
    name.w = W

#define V4_colori(name, R, G, B, A)                                                                \
    name.r = R;                                                                                    \
    name.g = G;                                                                                    \
    name.b = B;                                                                                    \
    name.a = A

#define V4_arrd(name, a0, a1, a2, a3)                                                              \
    v4 name = {0};                                                                                 \
    V4_colori(name, a0, a1, a2, a3)

#define V4_vecd(name, X, Y, Z, W)                                                                  \
    v4 name = {0};                                                                                 \
    V4_colori(name, X, Y, Z, W)

#define V4_colord(name, R, G, B, A)                                                                \
    v4 name = {0};                                                                                 \
    V4_colori(name, R, G, B, A)
//  (section) --------------------------------------------------------------------- : vec4(math)  //



//  quat(math) : --------------------------------------------------------------------- (section)  //
typedef union {
    f32 arr[4];
    struct {
        f32 q0, q1, q2, q3;
    };
    struct {
        f32 w, x, y, z;
    };
} quat;
StaticAssert(sizeof(quat) == (sizeof(f32) * 4), quat_size_check_eq_4);

#define Quat_arri(name, a0, a1, a2, a3)                                                            \
    name.arr[0] = a0;                                                                              \
    name.arr[1] = a1;                                                                              \
    name.arr[2] = a2;                                                                              \
    name.arr[3] = a3


#define Quat_i(name, q0, q1, q2, q3)                                                               \
    name.q0 = q0;                                                                                  \
    name.q1 = q1;                                                                                  \
    name.q2 = q2;                                                                                  \
    name.q3 = q3

#define Quat_arrd(name, a0, a1, a2, a3)                                                            \
    quat name = {0};                                                                               \
    Quat_arri(name, a0, a1, a2, a3)

#define Quat_d(name, q0, q1, q2, q3)                                                               \
    quat name = {0};                                                                               \
    Quat_i(name, q0, q1, q2, q3)
//  (section) --------------------------------------------------------------------- : quat(math)  //



//  vec3(math) : --------------------------------------------------------------------- (section)  //
typedef union {
    f32 arr[3];
    struct {
        f32 x, y, z;
    };
    struct {
        f32 r, g, b;
    };
} v3;
StaticAssert(sizeof(v3) == (sizeof(f32) * 3), v3_size_check_eq_3);

#define V3_arri(name, a0, a1, a2)                                                                  \
    name.arr[0] = a0;                                                                              \
    name.arr[1] = a1;                                                                              \
    name.arr[2] = a2;

#define V3_veci(name, X, Y, Z)                                                                     \
    name.x = X;                                                                                    \
    name.y = Y;                                                                                    \
    name.z = Z;

#define V3_colori(name, R, G, B)                                                                   \
    name.r = R;                                                                                    \
    name.g = G;                                                                                    \
    name.b = B;

#define V3_arrd(name, a0, a1, a2)                                                                  \
    v3 name = {};                                                                                  \
    V3_colori(name, a0, a1, a2)

#define V3_vecd(name, X, Y, Z)                                                                     \
    v3 name = {};                                                                                  \
    V3_colori(name, X, Y, Z)

#define V3_colord(name, R, G, B)                                                                   \
    v3 name = {};                                                                                  \
    V3_colori(name, R, G, B)
//  (section) --------------------------------------------------------------------- : vec3(math)  //


//  mat3(math) : --------------------------------------------------------------------- (section)  //
typedef union {
    f32 arr[9];
    f32 m[3][3];
    struct {
        v3 vec1;
        v3 vec2;
        v3 vec3;
    };
    v3 rows[3];
} mat3;
StaticAssert(sizeof(mat3) == sizeof(f32) * (9), mat3_size_check_eq_9);


#define Mat3_di(name)                                                                              \
    mat3 name    = {};                                                                             \
    name.m[0][0] = 1.0f;                                                                           \
    name.m[1][1] = 1.0f;                                                                           \
    name.m[2][2] = 1.0f;

#define Mat3_ii(name)                                                                              \
    name.m[0][0] = 1.0f;                                                                           \
    name.m[1][1] = 1.0f;                                                                           \
    name.m[2][2] = 1.0f;


//  (section) --------------------------------------------------------------------- : mat3(math)  //

//  mat4(math) : --------------------------------------------------------------------- (section)  //

typedef union {
    f32 arr[16];
    f32 m[4][4];
    struct {
        v3  vec1;
        f32 x;
        v3  vec2;
        f32 y;
        v3  vec3;
        f32 z;
        v3  vec4;
        f32 w;
    };
    v4 rows[4];
} mat4;
StaticAssert(sizeof(mat4) == sizeof(f32) * 16, mat4_size_check_eq_16);

#define Mat4_di(name)                                                                              \
    mat4 name    = {0};                                                                            \
    name.m[0][0] = 1.0f;                                                                           \
    name.m[1][1] = 1.0f;                                                                           \
    name.m[2][2] = 1.0f;                                                                           \
    name.m[3][3] = 1.0f

#define Mat4_ii(name)                                                                              \
    name.m[0][0] = 1.0f;                                                                           \
    name.m[1][1] = 1.0f;                                                                           \
    name.m[2][2] = 1.0f;                                                                           \
    name.m[3][3] = 1.0f


//  (section) --------------------------------------------------------------------- : mat4(math)  //



typedef struct {
    f32 x;
    f32 y;
    f32 w;
    f32 h;
} rectangle;

//  (section) -------------------------------------------------------------------- : apes stdlib  //
#endif
