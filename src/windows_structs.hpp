#ifndef WIN_LAYER_STRUCTS_H
#define WIN_LAYER_STRUCTS_H
// Structures Used by the Windows Layer -------------------------------------------------------- //

#include "canvas_sugars.hpp"
#include "canvas.hpp"

#include <windows.h>
#include <xinput.h>
#include <dsound.h>
#include <malloc.h>

typedef struct {
    BITMAPINFO Info;
    void*      Memory;
    uint32     Width;
    uint32     Height;
    uint32     Size;
    uint32     Pitch;
    uint32     BytesPerPixel;
} WinPlatBitMap;

typedef struct {
    uint32 Width;
    uint32 Height;
} WinPlatDimensions;

typedef struct {
    HMODULE                   game_lib;
    canvas_update_and_render* UpdateAndRender;
    bool                      is_valid;
    FILETIME                  last_write_time;
} WinPlatGameCode;

typedef struct {
    uint32       count;
    CanvasInput* input_stream;
} WinPlatInputRecord;

// --------------------------------------------------------------------------------------------- //
#endif
