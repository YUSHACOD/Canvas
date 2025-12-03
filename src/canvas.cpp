#include "canvas.hpp"
#include "canvas_sugars.hpp"

#include <math.h>


internal void
CanvasWeirdRender(CanvasBitMap *BitMap, uint32 XOff, uint32 YOff)
{

    uint8 *Row = (uint8 *)BitMap->Memory;

    for (uint32 Y = 0; Y < BitMap->Height; Y += 1)
    {

        uint32 *Pixel = (uint32 *)Row;
        for (uint32 X = 0; X < BitMap->Width; X += 1)
        {

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
                      CanvasInput *Input,
                      bool *Running)
{

#ifdef DEBUG
    Assert(sizeof(CanvasState) <= Memmory->PermaSize);
#endif

    CanvasState *State = (CanvasState *)Memmory->PermaStore;
    if (!Memmory->IsValid)
    {

#if 0
        char *FileName = Text(__FILE__);
        DEBUGFileStruct File = DEBUGPlatformReadEntireFile(FileName);
        if (File.Memory)
        {
            DEBUGPlatformWriteEntireFile(Text("./rama.txt"), File.Memory, (uint32)File.Size);
            DEBUGPlatformFreeFileMemory(File.Memory);
        }
#endif


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


    if (Input1->Stop.EndedDown)
    {
        *Running = false;
    }


    // Todo: Allow sample offsets here for more robust platform options.
    CanvasWeirdRender(BitMap, State->XOff, State->YOff);
}
