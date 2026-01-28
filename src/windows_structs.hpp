#ifndef WIN_LAYER_STRUCTS_H
#define WIN_LAYER_STRUCTS_H
// Structures Used by the Windows Layer -------------------------------------------------------- //

#include "sugars.hpp"
#include "canvas.hpp"

#include <windows.h>
#include <xinput.h>
#include <dsound.h>
#include <malloc.h>

typedef struct {
    BITMAPINFO Info;
    void*      Memory;
    u32        Width;
    u32        Height;
    u32        Size;
    u32        Pitch;
    u32        BytesPerPixel;
} WinPlatBitMap;

typedef struct {
    u32 Width;
    u32 Height;
} WinPlatDimensions;

typedef struct {
    HMODULE                   game_lib;
    canvas_update_and_render* UpdateAndRender;
    bool                      is_valid;
    FILETIME                  last_write_time;
} WinPlatGameCode;

typedef struct {
    u32          count;
    CanvasInput* input_stream;
} WinPlatInputRecord;

// --------------------------------------------------------------------------------------------- //
#endif
