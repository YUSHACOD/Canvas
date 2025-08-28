#include <math.h>

#include "canvas.hpp"
#include "canvas_sugars.hpp"

internal void
CanvasOutputSound(CanvasSoundBuffer *SoundBuffer, int32 ToneHz) {

    local_persist real32 tSine;

    int16 ToneVolume = 5000;
    int32 WavePeriod = SoundBuffer->SamplesPerSecond / ToneHz;

    int16 *SampleOut = SoundBuffer->SampleOut;

    for (int32 SampleIdx = 0; SampleIdx < SoundBuffer->SampleCount; SampleIdx += 1) {

        real32 SineValue = sinf(tSine);

        int16 SampleValue = (int16)(SineValue * (real32)ToneVolume);

        *SampleOut = SampleValue;
        SampleOut += 1;

        *SampleOut = SampleValue;
        SampleOut += 1;

        tSine += (1.0f * Pi32 * 2.0f) / (real32)WavePeriod;
    }
}

internal void
CanvasWeirdRender(CanvasBitMap *BitMap, int32 XOff, int32 YOff) {

    uint8 *Row = (uint8 *)BitMap->Memory;

    for (int32 Y = 0; Y < BitMap->Height; Y += 1) {

        uint32 *Pixel = (uint32 *)Row;
        for (int X = 0; X < BitMap->Width; X += 1) {

            // Blue
            uint8 blue = (uint8)(X + XOff);
            uint8 green = (uint8)(Y + YOff);
            uint8 red = 0;
            uint8 pad = 0;

            *Pixel =
                ((uint32)pad << 24) | ((uint32)red << 16) | ((uint32)green << 8) | ((uint32)blue);

            Pixel += 1;
        }

        Row += BitMap->Pitch;
    }
}


internal void
CanvasUpdateAndRender(CanvasBitMap *BitMap, int32 XOff, int32 YOff,
                      CanvasSoundBuffer *SoundBuffer, int32 ToneHz) {
    // Todo: Allow sample offsets here for more robust platform options.
    CanvasOutputSound(SoundBuffer, ToneHz);
    CanvasWeirdRender(BitMap, XOff, YOff);
}
