#ifndef WIN_LAYER_STRUCTS_H
#define WIN_LAYER_STRUCTS_H
// Structures Used by the Windows Layer -------------------------------------------------------- //

#include "canvas_sugars.hpp"
#include "canvas.cpp"

#include <windows.h>
#include <xinput.h>
#include <dsound.h>
#include <malloc.h>

typedef struct {
    BITMAPINFO Info;
    void *Memory;
    uint32 Width;
    uint32 Height;
    uint32 Size;
    uint32 Pitch;
    uint32 BytesPerPixel;
} WinPlatBitMap;

typedef struct {
    uint32 Width;
    uint32 Height;
} WinPlatDimensions;


// --------------------------------------------------------------------------------------------- //
#endif
