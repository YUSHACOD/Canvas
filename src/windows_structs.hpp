#ifndef WIN_LAYER_H
#define WIN_LAYER_H
// Structures Used by the Windows Layer -------------------------------------------------------- //


#include <stdio.h>
#include <math.h>
#include <windows.h>
#include <xinput.h>
#include <dsound.h>
#include <malloc.h>

#include "base/sugars.hpp"
#include "canvas.hpp"
#include "windows_opengl.hpp"

#define OPENGL 1

typedef struct {
    BITMAPINFO info;
    void*      memory;
    u32        width;
    u32        height;
    u32        size;
    u32        pitch;
    u32        bytes_per_pixel;
} WinPlatOffScreenBuffer;

typedef struct {
    u32 width;
    u32 height;
} WinPlatDimensions;

typedef struct {
    HMODULE                   game_lib;
    canvas_update_and_render* update_and_render;
    bool                      is_valid;
    FILETIME                  last_write_time;
} WinPlatGameCode;

typedef struct {
    u32          count;
    CanvasInput* input_stream;
} WinPlatInputRecord;

typedef struct {
    i64 counter;
    u64 cycle_count;
} WinPlatTimeCounter;


// --------------------------------------------------------------------------------------------- //
#endif
