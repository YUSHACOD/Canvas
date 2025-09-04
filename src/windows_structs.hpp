#ifndef WIN_LAYER_STRUCTS_H
#define WIN_LAYER_STRUCTS_H
// Structures Used by the Windows Layer -------------------------------------------------------- //

#include "canvas_sugars.hpp"
#include "canvas.cpp"

#include <stdio.h>
#include <windows.h>
#include <Xinput.h>
#include <dsound.h>
#include <malloc.h>

typedef struct {
    BITMAPINFO Info;
    void *Memory;
    int32 Width;
    int32 Height;
    int32 Size;
    int32 Pitch;
    int32 BytesPerPixel;
} WinPlatBitMap;

typedef struct {
    int32 Width;
    int32 Height;
} WinPlatDimensions;

typedef struct {
    int32 Channels;
    int32 SamplesPerSec;
    int32 BytesPerSample;
    int32 BufferSize;
    uint32 RunningSampleIndex;
    int32 LatencySampleCount;
} WinPlatSound;

// --------------------------------------------------------------------------------------------- //
#endif
