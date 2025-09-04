#include "canvas.hpp"
#include "canvas_sugars.hpp"

#include <math.h>

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
CanvasUpdateAndRender(CanvasMemmory *Memmory,
                      CanvasBitMap *BitMap,
                      CanvasSound *Sound,
                      CanvasInput *Input,
                      bool *Running) {

#ifdef DEBUG
    Assert(sizeof(CanvasState) <= Memmory->PermaSize);
#endif

    CanvasState *State = (CanvasState *)Memmory->PermaStore;
    if (!Memmory->IsValid) {

#ifdef DEBUG
        char *FileName = Text(__FILE__);
        DEBUGFileStruct File = DEBUGPlatformReadEntireFile(FileName);
        if (File.Memory) {
            DEBUGPlatformWriteEntireFile(Text("./rama.txt"), File.Memory, (uint32)File.Size);
            DEBUGPlatformFreeFileMemory(File.Memory);
        }
#endif


        State->ToneHz = 256;
        Memmory->IsValid = true;
    }

    // Input Handling ------------------------------------------ //
    CanvasControllerInput *Input1 = &Input->Controllers[0];

    State->XOff += (int32)(4.0f * Input1->LeftStickX.End);
    State->YOff -= (int32)(4.0f * Input1->LeftStickY.End);

    State->XOff -= (Input1->Left.EndedDown) ? 2 : 0;
    State->XOff += (Input1->Right.EndedDown) ? 2 : 0;

    State->YOff -= (Input1->Up.EndedDown) ? 2 : 0;
    State->YOff += (Input1->Down.EndedDown) ? 2 : 0;

    State->ToneHz += (Input1->RS.EndedDown) ? 1 : 0;
    State->ToneHz -= (Input1->LS.EndedDown) ? 1 : 0;

    if (Input1->Stop.EndedDown) {
        *Running = false;
    }

    int32 ExtraToneHz = (int32)(Input1->RightTrigger.End * 256.0f);

    int32 ToneVolume = (int32)(3000.0f * Input1->LeftTrigger.End);

    // Todo: Allow sample offsets here for more robust platform options.
    CanvasOutputSound(Sound, ToneVolume, State->ToneHz, ExtraToneHz);
    CanvasWeirdRender(BitMap, State->XOff, State->YOff);
}
