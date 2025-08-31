#include <math.h>

#include "canvas.hpp"
#include "canvas_sugars.hpp"

internal void
CanvasOutputSound(CanvasSound *Sound, int32 ToneVolume, int32 ToneHz, int32 ExtraToneHz) {

    local_persist real32 tSine;

    int32 WavePeriod = Sound->SamplesPerSecond / (ToneHz + ExtraToneHz);

    int16 *SampleOut = Sound->SampleOut;

    for (int32 SampleIdx = 0; SampleIdx < Sound->SampleCount; SampleIdx += 1) {

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
CanvasUpdateAndRender(CanvasBitMap *BitMap, CanvasSound *Sound, CanvasInput *Input, bool *Running) {

    local_persist int32 XOff = 0;
    local_persist int32 YOff = 0;
    local_persist int32 ToneHz = 256;

    // Input Handling ------------------------------------------ //
    CanvasControllerInput *Input1 = &Input->Controllers[0];

    XOff += (int32)(4.0f * Input1->LeftStickX.End);
    YOff -= (int32)(4.0f * Input1->LeftStickY.End);

    XOff -= (Input1->Left.EndedDown) ? 2 : 0;
    XOff += (Input1->Right.EndedDown) ? 2 : 0;

    YOff -= (Input1->Up.EndedDown) ? 2 : 0;
    YOff += (Input1->Down.EndedDown) ? 2 : 0;

    ToneHz += (Input1->RS.EndedDown) ? 1 : 0;
    ToneHz -= (Input1->LS.EndedDown) ? 1 : 0;

    *Running = !(Input1->Stop.EndedDown);

    // WinCanvasSound.WavePeriod =
    //     WinCanvasSound.SamplesPerSec / (WinCanvasSound.ToneHz +
    //     RightTrigger);
    int32 ExtraToneHz = (int32)(Input1->RightTrigger.End * 256.0f);

    // WinCanvasSound.ToneVolume =
    //     (int32)((3000.0f) * ((real32)LeftTrigger / 255.0f));
    // --------------------------------------------------------- //
    int32 ToneVolume = (int32)(3000.0f * Input1->LeftTrigger.End);

    // Todo: Allow sample offsets here for more robust platform options.
    CanvasOutputSound(Sound, ToneVolume, ToneHz, ExtraToneHz);
    CanvasWeirdRender(BitMap, XOff, YOff);
}
