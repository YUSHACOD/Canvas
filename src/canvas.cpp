#include "canvas.hpp"
#include "canvas_sugars.hpp"

#include <math.h>

typedef struct {
    uint8 red;
    uint8 green;
    uint8 blue;
    uint8 pad;
} Color;

internal inline void DrawPixel(uint32 *Pixel, Color color) {
    *Pixel = ((uint32)color.pad << 24) | ((uint32)color.red << 16) | ((uint32)color.green << 8) |
             ((uint32)color.blue);
}

internal inline void DrawPixel(uint32 *Pixel, uint8 Red, uint8 Blue, uint8 Green, uint8 Pad) {
    *Pixel = ((uint32)Pad << 24) | ((uint32)Red << 16) | ((uint32)Green << 8) | ((uint32)Blue);
}

internal void DrawSquare(CanvasBitMap *BitMap, uint32 X, uint32 Y, Color c, uint32 size) {

    if ((X < BitMap->Width) && (Y < BitMap->Height)) {

        uint32 draw_width =
            ((X + size) > BitMap->Width) ? (BitMap->Width - X) % BitMap->Width : (size);
        uint32 draw_height =
            ((Y + size) > BitMap->Height) ? (BitMap->Height - Y) % BitMap->Height : (size);

        for (uint32 draw_y = 0; draw_y < draw_height; draw_y += 1) {
            for (uint32 draw_x = 0; draw_x < draw_width; draw_x += 1) {

                uint8  *draw_row   = ((uint8 *)BitMap->Memory) + (BitMap->Pitch * (Y + draw_y));
                uint32 *draw_pixel = ((uint32 *)draw_row + (X + draw_x));

                DrawPixel(draw_pixel, c);
            }
        }
    }
}

void CanvasWeirdRender(CanvasBitMap *BitMap, uint32 XOff, uint32 YOff) {

    uint8 *Row = (uint8 *)BitMap->Memory;

    for (uint32 Y = 0; Y < BitMap->Height; Y += 1) {
        uint32 *Pixel = (uint32 *)Row;
        for (uint32 X = 0; X < BitMap->Width; X += 1) {

            // Blue
            uint8 blue  = (uint8)(X + XOff);
            uint8 green = (uint8)(Y + YOff);
            uint8 red   = 0;
            uint8 pad   = 0;

            DrawPixel(Pixel, red, blue, green, pad);

            Pixel += 1;
        }

        Row += BitMap->Pitch;
    }
}

void JOY(CanvasBitMap *bitmap, CanvasState *state) {

    Color c = {};
    c.red   = 255;
    c.blue  = 244;

    DrawSquare(bitmap, 300 + state->XOff, 300 + state->YOff, c, 30);
}

internal void CanvasUpdateAndRender(CanvasMemmory *Memmory,
                                    CanvasBitMap  *BitMap,
                                    CanvasInput   *Input,
                                    bool          *Running) {

#ifdef DEBUG
    Assert(sizeof(CanvasState) <= Memmory->PermaSize);
#endif

    CanvasState *State = (CanvasState *)Memmory->PermaStore;
    if (!Memmory->IsValid) {

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


    if (Input1->Stop.EndedDown) {
        *Running = false;
    }


    // Todo: Allow sample offsets here for more robust platform options.
    // CanvasWeirdRender(BitMap, State->XOff, State->YOff);
    JOY(BitMap, State);
}
