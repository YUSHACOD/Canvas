#ifndef WIN_LAYER_H
#define WIN_LAYER_H
//  Windows interfaces : ------------------------------------------------------------- (section)  //

#include <windows.h>

#include <base/include.cpp>
#include "canvas_platform.hpp"


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
    HMODULE                 game_lib;
    canvas_game_init*       init;
    canvas_update_and_draw* update_and_draw;
    bool                    is_valid;
    FILETIME                last_write_time;
} winplat_game_code;

typedef struct {
    u32           count;
    canvas_input* input_stream;
} winplat_input_record;

typedef struct {
    i64 counter;
    u64 cycle_count;
} winplat_time_counter;

typedef struct {
    HWND                 window_handle;
    winplat_game_code    game_code;
    canvas_input         inputs[2];
    canvas_input*        old_input;
    canvas_input*        new_input;
    render_push_buffer   r_push_buffer;
    canvas_memory        memory;
    f64                  time_elapsed;
    winplat_time_counter last;
    i64                  perf_counter_freq;
    f64                  max_time_per_frame;
    bool                 is_time_proper;
} winplat_main_ctx;

//  (section) ------------------------------------------------------------- : Windows interfaces  //
#endif
