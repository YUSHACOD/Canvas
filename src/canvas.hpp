#ifndef CANVAS_H
#define CANVAS_H
// The Game Interface --------------------------------------------------------------------------- //

#include "canvas_sugars.hpp"

typedef struct {
    void *Memory;
    int32 Width;
    int32 Height;
    int32 Size;
    int32 Pitch;
    int32 BytesPerPixel;
} CanvasBitMap;

typedef struct {
    int16 *SampleOut;
    int32 SamplesPerSecond;
    int32 SampleCount;
} CanvasSoundBuffer;

internal void CanvasUpdateAndRender(CanvasBitMap *BitMap, int32 XOff, int32 YOff, CanvasSoundBuffer *SoundBuffer, int32 ToneHz);

// ---------------------------------------------------------------------------------------------- //
#endif
