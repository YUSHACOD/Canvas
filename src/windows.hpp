#ifndef WIN_LAYER_H
#define WIN_LAYER_H
// Structures Used by the Windows Layer -------------------------------------------------------- //


#include <math.h>
#include <windows.h>
#include <xinput.h>
#include <dsound.h>
#include <malloc.h>

#include "base/sugars.hpp"
#include "canvas.hpp"

#define OPENGL 1

typedef struct {
    BITMAPINFO info;
    void*      memory;
    u32        width;
    u32        height;
    u32        size;
    u32        pitch;
    u32        bytes_per_pixel;
} winplat_off_screen_buffer;

typedef struct {
    u32 width;
    u32 height;
} winplat_dimensions;

typedef struct {
    HMODULE                   game_lib;
    canvas_update_and_render* update_and_render;
    bool                      is_valid;
    FILETIME                  last_write_time;
} winplat_game_code;

typedef struct {
    u32          count;
    canvas_input* input_stream;
} winplat_input_record;

typedef struct {
    i64 counter;
    u64 cycle_count;
} winplat_time_counter;

// --------------------------------------------------------------------------------------------- //
#endif
