#include "canvas.hpp"
#include "canvas_sugars.hpp"


internal void
CanvasUpdateAndRender(CanvasBitMap BitMap, int32 XOff, int32 YOff) {

    uint8 *Row = (uint8 *)BitMap.Memory;

    for (int32 Y = 0; Y < BitMap.Height; Y += 1) {

        uint32 *Pixel = (uint32 *)Row;
        for (int X = 0; X < BitMap.Width; X += 1) {

            // Blue
            uint8 blue = (uint8)(X + XOff);
            uint8 green = (uint8)(Y + YOff);
            uint8 red = 0;
            uint8 pad = 0;

            *Pixel =
                ((uint32)pad << 24) | ((uint32)red << 16) | ((uint32)green << 8) | ((uint32)blue);

            Pixel += 1;
        }

        Row += BitMap.Pitch;
    }
}
