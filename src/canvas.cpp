
#include "canvas.hpp"
#include "canvas_sugars.hpp"


typedef struct {
    uint8 red;
    uint8 green;
    uint8 blue;
    uint8 pad;
} Color;

inline internal void DrawPixel(uint32 *draw_pixel, uint8 red, uint8 green, uint8 blue, uint8 pad) {
    *draw_pixel = ((uint32)pad << 24) | ((uint32)red << 16) | ((uint32)green << 8) | ((uint32)blue);
}

internal inline void
DrawPixel(CanvasBitMap *bitmap, int32 x, int32 y, uint8 red, uint8 blue, uint8 green, uint8 pad) {

    uint32 dx = x + (bitmap->Width / 2);
    uint32 dy = y + (bitmap->Height / 2);

    if ((dx < bitmap->Width) && (dy < bitmap->Height)) {

        uint8  *draw_row   = ((uint8 *)bitmap->Memory) + (bitmap->Pitch * dy);
        uint32 *draw_pixel = ((uint32 *)draw_row + dx);

        *draw_pixel =
            ((uint32)pad << 24) | ((uint32)red << 16) | ((uint32)green << 8) | ((uint32)blue);
    }
}

internal inline void DrawPixel(CanvasBitMap *bitmap, int32 x, int32 y, Color c) {

    uint32 dx = x + (bitmap->Width / 2);
    uint32 dy = y + (bitmap->Height / 2);

    if ((dx < bitmap->Width) && (dy < bitmap->Height)) {

        uint8  *draw_row   = ((uint8 *)bitmap->Memory) + (bitmap->Pitch * dy);
        uint32 *draw_pixel = ((uint32 *)draw_row + dx);

        *draw_pixel = ((uint32)c.pad << 24) | ((uint32)c.red << 16) | ((uint32)c.green << 8) |
                      ((uint32)c.blue);
    }
}

internal void
DrawRectangle(CanvasBitMap *bitmap, int32 x, int32 y, Color c, uint32 width, uint32 height) {

    for (uint32 draw_y = 0; draw_y < height; draw_y += 1) {
        for (uint32 draw_x = 0; draw_x < width; draw_x += 1) {
            DrawPixel(bitmap, draw_x + x, draw_y + y, c);
        }
    }
}

internal void DrawSquare(CanvasBitMap *bitmap, int32 x, int32 y, Color c, uint32 size) {
    DrawRectangle(bitmap, x, y, c, size, size);
}

inline internal void DrawSquareC(CanvasBitMap *BitMap, int32 X, int32 Y, Color c, uint32 size) {
    DrawSquare(BitMap, X - (size / 2), Y - (size / 2), c, size);
}

void BLine(CanvasBitMap *BitMap, int32 x0, int32 y0, int32 x1, int32 y1, Color c, uint32 width) {

    int32 dx = ((x1 - x0) > 0) ? (x1 - x0) : (x0 - x1);
    int32 dy = ((y1 - y0) > 0) ? (y0 - y1) : (y1 - y0);

	int32 sx = (x0 < x1) ? 1 : -1;
	int32 sy = (y0 < y1) ? 1 : -1;

	int32 e = dx + dy;

    while (true) {

        DrawSquareC(BitMap, x0, y0, c, width);

		if ((x0 == x1) && (y0 == y1)) {
			break;
		}

		int32 e2 = 2 * e;
		if (e2 >= dy) {
			if (x0 == x1) {
				break;
			}

			e = e + dy;
			x0 = x0 + sx;
		}
		
		if (e2 <= dx) {
			if (y0 ==  y1) {
				break;
			}

			e = e + dx;
			y0 = y0 + sy;
		}
    }
}

void CanvasWeirdRender(CanvasBitMap *BitMap, uint32 XOff, uint32 YOff) {

    uint8 *Row = (uint8 *)BitMap->Memory;

    for (uint32 Y = 0; Y < BitMap->Height; Y += 1) {
        uint32 *Pixel = (uint32 *)Row;
        for (uint32 X = 0; X < BitMap->Width; X += 1) {

            // Blue
            // uint8 blue  = (uint8)(X + XOff);
            // uint8 green = (uint8)(Y + YOff);
            uint8 blue  = 255;
            uint8 green = 255;
            uint8 red   = 255;
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

    DrawSquareC(bitmap, state->XOff, state->YOff, c, 30);

    DrawSquareC(bitmap, 0, 0, c, 20);
    DrawSquareC(bitmap, -960, -540, c, 20);
    DrawSquareC(bitmap, -960, 0, c, 20);
    DrawSquareC(bitmap, 959, 0, c, 20);
    DrawSquareC(bitmap, 0, -540, c, 20);
    DrawSquareC(bitmap, 0, 539, c, 20);
    DrawSquareC(bitmap, -960, 539, c, 20);
    DrawSquareC(bitmap, 959, -540, c, 20);
    DrawSquareC(bitmap, 959, 539, c, 20);

	DrawSquareC(bitmap, -400, -100, c, 10);
    BLine(bitmap, 0, 0, -400, -100, c, 5);


    c.blue = 0;

    // bugs in drawing squares in the corners
    // DrawSquare(bitmap, 0, 0, c, 20);
    // DrawSquare(bitmap, 0, 1040, c, 20);
    // DrawSquare(bitmap, 1900, 0, c, 20);
    // DrawSquare(bitmap, 1900, 1040, c, 20);
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
