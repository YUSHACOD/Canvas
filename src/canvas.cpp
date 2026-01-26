
#include "canvas.hpp"
#include "canvas_sugars.hpp"


typedef struct {
    uint8 red;
    uint8 green;
    uint8 blue;
    uint8 pad;
} Color;

inline internal void DrawPixel(uint32* draw_pixel, uint8 red, uint8 green, uint8 blue, uint8 pad) {
    *draw_pixel = ((uint32)pad << 24) | ((uint32)red << 16) | ((uint32)green << 8) | ((uint32)blue);
}

internal inline void
DrawPixel(CanvasBitMap* bitmap, int32 x, int32 y, uint8 red, uint8 blue, uint8 green, uint8 pad) {

    uint32 dx = x + (bitmap->Width / 2);
    uint32 dy = y + (bitmap->Height / 2);

    if ((dx < bitmap->Width) && (dy < bitmap->Height)) {

        uint8*  draw_row   = ((uint8*)bitmap->Memory) + (bitmap->Pitch * dy);
        uint32* draw_pixel = ((uint32*)draw_row + dx);

        *draw_pixel =
            ((uint32)pad << 24) | ((uint32)red << 16) | ((uint32)green << 8) | ((uint32)blue);
    }
}

internal inline void DrawPixel(CanvasBitMap* bitmap, int32 x, int32 y, Color c) {

    uint32 dx = x + (bitmap->Width / 2);
    uint32 dy = y + (bitmap->Height / 2);

    if ((dx < bitmap->Width) && (dy < bitmap->Height)) {

        uint8*  draw_row   = ((uint8*)bitmap->Memory) + (bitmap->Pitch * dy);
        uint32* draw_pixel = ((uint32*)draw_row + dx);

        *draw_pixel = ((uint32)c.pad << 24) | ((uint32)c.red << 16) | ((uint32)c.green << 8) |
                      ((uint32)c.blue);
    }
}

internal void
DrawRectangle(CanvasBitMap* bitmap, int32 x, int32 y, Color c, uint32 width, uint32 height) {

    for (uint32 draw_y = 0; draw_y < height; draw_y += 1) {
        for (uint32 draw_x = 0; draw_x < width; draw_x += 1) {
            DrawPixel(bitmap, draw_x + x, draw_y + y, c);
        }
    }
}

internal void DrawSquare(CanvasBitMap* bitmap, int32 x, int32 y, Color c, uint32 size) {
    DrawRectangle(bitmap, x, y, c, size, size);
}

inline internal void DrawSquareC(CanvasBitMap* BitMap, int32 X, int32 Y, Color c, uint32 size) {
    DrawSquare(BitMap, X - (size / 2), Y - (size / 2), c, size);
}

void CaseyCircle(CanvasBitMap* BitMap, int32 Cx, int32 Cy, uint32 r, Color c) {
    int r2 = r + r;

    int X  = r;
    int Y  = 0;
    int dY = -2;
    int dX = r2 + r2 - 4;
    int D  = r2 - 1;

    while (Y <= X) {

        DrawPixel(BitMap, Cx - X, Cy - Y, c);
        DrawPixel(BitMap, Cx + X, Cy - Y, c);
        DrawPixel(BitMap, Cx - X, Cy + Y, c);
        DrawPixel(BitMap, Cx + X, Cy + Y, c);
        DrawPixel(BitMap, Cx - Y, Cy - X, c);
        DrawPixel(BitMap, Cx + Y, Cy - X, c);
        DrawPixel(BitMap, Cx - Y, Cy + X, c);
        DrawPixel(BitMap, Cx + Y, Cy + X, c);

        D += dY;
        dY -= 4;
        ++Y;

        if (D < 0) {
            D += dX;
            dX -= 4;
            --X;
        }
    }
}

void FillLine(CanvasBitMap* bitmap, int32 Cx, int32 Cy, int32 x, int32 y, Color c) {
    for (int32 i = y; i >= -y; i -= 1) {
        DrawPixel(bitmap, Cx - x, Cy - i, c);
    }
}

void CaseyCircleFill(CanvasBitMap* BitMap, int32 Cx, int32 Cy, uint32 r, Color c) {
    int r2 = r + r;

    int X  = r;
    int Y  = 0;
    int dY = -2;
    int dX = r2 + r2 - 4;
    int D  = r2 - 1;

    while (Y <= X) {

        FillLine(BitMap, Cx, Cy, -X, Y, c);
        FillLine(BitMap, Cx, Cy, -Y, X, c);
        FillLine(BitMap, Cx, Cy, X, Y, c);
        FillLine(BitMap, Cx, Cy, Y, X, c);

        D += dY;
        dY -= 4;
        ++Y;

        if (D < 0) {
            D += dX;
            dX -= 4;
            --X;
        }
    }
}

void BLine(CanvasBitMap* BitMap, int32 x0, int32 y0, int32 x1, int32 y1, Color c, uint32 width) {

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

            e  = e + dy;
            x0 = x0 + sx;
        }

        if (e2 <= dx) {
            if (y0 == y1) {
                break;
            }

            e  = e + dx;
            y0 = y0 + sy;
        }
    }
}

void CanvasWeirdRender(CanvasBitMap* BitMap, uint32 XOff, uint32 YOff) {

    uint8* Row = (uint8*)BitMap->Memory;

    for (uint32 Y = 0; Y < BitMap->Height; Y += 1) {
        uint32* Pixel = (uint32*)Row;
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

void JOY(CanvasBitMap* bitmap, CanvasState* state) {

    Color c = {};
    c.red   = 255;
    c.blue  = 0;
    c.green = 255;


    // DrawSquareC(bitmap, state->XOff, state->YOff, c, 30);

    // DrawSquareC(bitmap, 0, 0, c, 20);
    // DrawSquareC(bitmap, -960, -540, c, 20);
    // DrawSquareC(bitmap, -960, 0, c, 20);
    // DrawSquareC(bitmap, 959, 0, c, 20);
    // DrawSquareC(bitmap, 0, -540, c, 20);
    // DrawSquareC(bitmap, 0, 539, c, 20);
    // DrawSquareC(bitmap, -960, 539, c, 20);
    // DrawSquareC(bitmap, 959, -540, c, 20);
    // DrawSquareC(bitmap, 959, 539, c, 20);


	CaseyCircleFill(bitmap, state->XOff, state->YOff, 30, c);
	c.red = 100;
	c.blue = 200;
	c.green = 0;
    BLine(bitmap, 0, 0, state->XOff, state->YOff, c, 5);

    // CaseyCircleFill(bitmap, state->XOff + state->JX, state->YOff + state->JY, state->Weight, c);
}


extern "C" CANVAS_UPDATE_AND_RENDER(CanvasUpdateAndRender) {

#ifdef DEBUG
    Assert(sizeof(CanvasState) <= Memory->PermaSize);
#endif

    CanvasState* State = (CanvasState*)Memory->PermaStore;

    if (!Memory->IsValid) {

#if 0
        char*          FileName = Text(__FILE__);
        DBG_FileStruct File     = Memory->DBG_PlatReadEntireFile(FileName);
        if (File.Memory) {
            Memory->DBG_PlatWriteEntireFile(Text("./rama.txt"), File.Memory, (uint32)File.Size);
            Memory->DBG_PlatFreeFileMemory(File.Memory);
        }
#endif

        Memory->IsValid = true;
    }

    // Input Handling ------------------------------------------ //
    CanvasControllerInput* Input1 = &Input->Controllers[0];

    State->JX = (int32)(200.0f * Input1->LeftStickX.End);
    State->JY = (int32)(-200.0f * Input1->LeftStickY.End);

    State->XOff -= (Input1->Left.EndedDown) ? 2 : 0;
    State->XOff += (Input1->Right.EndedDown) ? 2 : 0;

    State->YOff -= (Input1->Up.EndedDown) ? 2 : 0;
    State->YOff += (Input1->Down.EndedDown) ? 2 : 0;

    State->Weight += (uint32)(5.0f * Input1->RightTrigger.End);
    State->Weight -= (uint32)(5.0f * Input1->LeftTrigger.End);

    if (Input1->Stop.EndedDown) {
        *Running = false;
    }

    State->XOff -= (Input->Keyboard.A.EndedDown) ? 10 : 0;
    State->XOff += (Input->Keyboard.D.EndedDown) ? 10 : 0;

    State->YOff -= (Input->Keyboard.W.EndedDown) ? 10 : 0;
    State->YOff += (Input->Keyboard.S.EndedDown) ? 10 : 0;

    if (Input->Keyboard.Alt.EndedDown && Input->Keyboard.B.EndedDown) {
        State->Weight += 5;
    }

    if (Input->Keyboard.Control.EndedDown && Input->Keyboard.R.EndedDown) {
		ZeroMemory(State, sizeof(CanvasState));
	}



    // Todo: Allow sample offsets here for more robust platform options.
    // CanvasWeirdRender(BitMap, State->XOff, State->YOff);
    JOY(BitMap, State);
}
